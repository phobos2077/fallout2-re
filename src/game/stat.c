#include "game/stat.h"

#include <stdio.h>

#include "game/combat.h"
#include "core.h"
#include "game/critter.h"
#include "game/display.h"
#include "game/game.h"
#include "game/gsound.h"
#include "game/intface.h"
#include "game/item.h"
#include "memory.h"
#include "game/object.h"
#include "game/perk.h"
#include "game/proto.h"
#include "game/roll.h"
#include "game/scripts.h"
#include "game/skill.h"
#include "tile.h"
#include "trait.h"

// 0x51D53C
StatDescription gStatDescriptions[STAT_COUNT] = {
    { NULL, NULL, 0, PRIMARY_STAT_MIN, PRIMARY_STAT_MAX, 5 },
    { NULL, NULL, 1, PRIMARY_STAT_MIN, PRIMARY_STAT_MAX, 5 },
    { NULL, NULL, 2, PRIMARY_STAT_MIN, PRIMARY_STAT_MAX, 5 },
    { NULL, NULL, 3, PRIMARY_STAT_MIN, PRIMARY_STAT_MAX, 5 },
    { NULL, NULL, 4, PRIMARY_STAT_MIN, PRIMARY_STAT_MAX, 5 },
    { NULL, NULL, 5, PRIMARY_STAT_MIN, PRIMARY_STAT_MAX, 5 },
    { NULL, NULL, 6, PRIMARY_STAT_MIN, PRIMARY_STAT_MAX, 5 },
    { NULL, NULL, 10, 0, 999, 0 },
    { NULL, NULL, 75, 1, 99, 0 },
    { NULL, NULL, 18, 0, 999, 0 },
    { NULL, NULL, 31, 0, INT_MAX, 0 },
    { NULL, NULL, 32, 0, 500, 0 },
    { NULL, NULL, 20, 0, 999, 0 },
    { NULL, NULL, 24, 0, 60, 0 },
    { NULL, NULL, 25, 0, 30, 0 },
    { NULL, NULL, 26, 0, 100, 0 },
    { NULL, NULL, 94, -60, 100, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 22, 0, 90, 0 },
    { NULL, NULL, 0, 0, 90, 0 },
    { NULL, NULL, 0, 0, 90, 0 },
    { NULL, NULL, 0, 0, 90, 0 },
    { NULL, NULL, 0, 0, 90, 0 },
    { NULL, NULL, 0, 0, 100, 0 },
    { NULL, NULL, 0, 0, 90, 0 },
    { NULL, NULL, 83, 0, 95, 0 },
    { NULL, NULL, 23, 0, 95, 0 },
    { NULL, NULL, 0, 16, 101, 25 },
    { NULL, NULL, 0, 0, 1, 0 },
    { NULL, NULL, 10, 0, 2000, 0 },
    { NULL, NULL, 11, 0, 2000, 0 },
    { NULL, NULL, 12, 0, 2000, 0 },
};

// 0x51D8CC
StatDescription gPcStatDescriptions[PC_STAT_COUNT] = {
    { NULL, NULL, 0, 0, INT_MAX, 0 },
    { NULL, NULL, 0, 1, PC_LEVEL_MAX, 1 },
    { NULL, NULL, 0, 0, INT_MAX, 0 },
    { NULL, NULL, 0, -20, 20, 0 },
    { NULL, NULL, 0, 0, INT_MAX, 0 },
};

// 0x66817C
MessageList gStatsMessageList;

// 0x668184
char* gStatValueDescriptions[PRIMARY_STAT_RANGE];

// 0x6681AC
int gPcStatValues[PC_STAT_COUNT];

// 0x4AED70
int statsInit()
{
    MessageListItem messageListItem;

    // NOTE: Uninline.
    pcStatsReset();

    if (!message_init(&gStatsMessageList)) {
        return -1;
    }

    char path[MAX_PATH];
    sprintf(path, "%s%s", msg_path, "stat.msg");

    if (!message_load(&gStatsMessageList, path)) {
        return -1;
    }

    for (int stat = 0; stat < STAT_COUNT; stat++) {
        gStatDescriptions[stat].name = getmsg(&gStatsMessageList, &messageListItem, 100 + stat);
        gStatDescriptions[stat].description = getmsg(&gStatsMessageList, &messageListItem, 200 + stat);
    }

    for (int pcStat = 0; pcStat < PC_STAT_COUNT; pcStat++) {
        gPcStatDescriptions[pcStat].name = getmsg(&gStatsMessageList, &messageListItem, 400 + pcStat);
        gPcStatDescriptions[pcStat].description = getmsg(&gStatsMessageList, &messageListItem, 500 + pcStat);
    }

    for (int index = 0; index < PRIMARY_STAT_RANGE; index++) {
        gStatValueDescriptions[index] = getmsg(&gStatsMessageList, &messageListItem, 301 + index);
    }

    return 0;
}

// 0x4AEEC0
int statsReset()
{
    // NOTE: Uninline.
    pcStatsReset();

    return 0;
}

// 0x4AEEE4
int statsExit()
{
    message_exit(&gStatsMessageList);

    return 0;
}

// 0x4AEEF4
int statsLoad(File* stream)
{
    for (int index = 0; index < PC_STAT_COUNT; index++) {
        if (fileReadInt32(stream, &(gPcStatValues[index])) == -1) {
            return -1;
        }
    }

    return 0;
}

// 0x4AEF20
int statsSave(File* stream)
{
    for (int index = 0; index < PC_STAT_COUNT; index++) {
        if (fileWriteInt32(stream, gPcStatValues[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

// 0x4AEF48
int critterGetStat(Object* critter, int stat)
{
    int value;
    if (stat >= 0 && stat < SAVEABLE_STAT_COUNT) {
        value = critterGetBaseStatWithTraitModifier(critter, stat);
        value += critterGetBonusStat(critter, stat);

        switch (stat) {
        case STAT_PERCEPTION:
            if ((critter->data.critter.combat.results & DAM_BLIND) != 0) {
                value -= 5;
            }
            break;
        case STAT_MAXIMUM_ACTION_POINTS:
            if (1) {
                int remainingCarryWeight = critterGetStat(critter, STAT_CARRY_WEIGHT) - item_total_weight(critter);
                if (remainingCarryWeight < 0) {
                    value -= -remainingCarryWeight / 40 + 1;
                }
            }
            break;
        case STAT_ARMOR_CLASS:
            if (isInCombat()) {
                if (combat_whose_turn() != critter) {
                    int actionPointsMultiplier = 1;
                    int hthEvadeBonus = 0;

                    if (critter == obj_dude) {
                        if (perkHasRank(obj_dude, PERK_HTH_EVADE)) {
                            bool hasWeapon = false;

                            Object* item2 = inven_right_hand(obj_dude);
                            if (item2 != NULL) {
                                if (item_get_type(item2) == ITEM_TYPE_WEAPON) {
                                    if (item_w_anim_code(item2) != WEAPON_ANIMATION_NONE) {
                                        hasWeapon = true;
                                    }
                                }
                            }

                            if (!hasWeapon) {
                                Object* item1 = inven_left_hand(obj_dude);
                                if (item1 != NULL) {
                                    if (item_get_type(item1) == ITEM_TYPE_WEAPON) {
                                        if (item_w_anim_code(item1) != WEAPON_ANIMATION_NONE) {
                                            hasWeapon = true;
                                        }
                                    }
                                }
                            }

                            if (!hasWeapon) {
                                actionPointsMultiplier = 2;
                                hthEvadeBonus = skill_level(obj_dude, SKILL_UNARMED) / 12;
                            }
                        }
                    }
                    value += hthEvadeBonus;
                    value += critter->data.critter.combat.ap * actionPointsMultiplier;
                }
            }
            break;
        case STAT_AGE:
            value += game_time() / GAME_TIME_TICKS_PER_YEAR;
            break;
        }

        if (critter == obj_dude) {
            switch (stat) {
            case STAT_STRENGTH:
                if (perk_level(critter, PERK_GAIN_STRENGTH)) {
                    value++;
                }

                if (perk_level(critter, PERK_ADRENALINE_RUSH)) {
                    if (critterGetStat(critter, STAT_CURRENT_HIT_POINTS) < (critterGetStat(critter, STAT_MAXIMUM_HIT_POINTS) / 2)) {
                        value++;
                    }
                }
                break;
            case STAT_PERCEPTION:
                if (perk_level(critter, PERK_GAIN_PERCEPTION)) {
                    value++;
                }
                break;
            case STAT_ENDURANCE:
                if (perk_level(critter, PERK_GAIN_ENDURANCE)) {
                    value++;
                }
                break;
            case STAT_CHARISMA:
                if (1) {
                    if (perk_level(critter, PERK_GAIN_CHARISMA)) {
                        value++;
                    }

                    bool hasMirrorShades = false;

                    Object* item2 = inven_right_hand(critter);
                    if (item2 != NULL && item2->pid == PROTO_ID_MIRRORED_SHADES) {
                        hasMirrorShades = true;
                    }

                    Object* item1 = inven_left_hand(critter);
                    if (item1 != NULL && item1->pid == PROTO_ID_MIRRORED_SHADES) {
                        hasMirrorShades = true;
                    }

                    if (hasMirrorShades) {
                        value++;
                    }
                }
                break;
            case STAT_INTELLIGENCE:
                if (perk_level(critter, PERK_GAIN_INTELLIGENCE)) {
                    value++;
                }
                break;
            case STAT_AGILITY:
                if (perk_level(critter, PERK_GAIN_AGILITY)) {
                    value++;
                }
                break;
            case STAT_LUCK:
                if (perk_level(critter, PERK_GAIN_LUCK)) {
                    value++;
                }
                break;
            case STAT_MAXIMUM_HIT_POINTS:
                if (perk_level(critter, PERK_ALCOHOL_RAISED_HIT_POINTS)) {
                    value += 2;
                }

                if (perk_level(critter, PERK_ALCOHOL_RAISED_HIT_POINTS_II)) {
                    value += 4;
                }

                if (perk_level(critter, PERK_ALCOHOL_LOWERED_HIT_POINTS)) {
                    value -= 2;
                }

                if (perk_level(critter, PERK_ALCOHOL_LOWERED_HIT_POINTS_II)) {
                    value -= 4;
                }

                if (perk_level(critter, PERK_AUTODOC_RAISED_HIT_POINTS)) {
                    value += 2;
                }

                if (perk_level(critter, PERK_AUTODOC_RAISED_HIT_POINTS_II)) {
                    value += 4;
                }

                if (perk_level(critter, PERK_AUTODOC_LOWERED_HIT_POINTS)) {
                    value -= 2;
                }

                if (perk_level(critter, PERK_AUTODOC_LOWERED_HIT_POINTS_II)) {
                    value -= 4;
                }
                break;
            case STAT_DAMAGE_RESISTANCE:
            case STAT_DAMAGE_RESISTANCE_EXPLOSION:
                if (perk_level(critter, PERK_DERMAL_IMPACT_ARMOR)) {
                    value += 5;
                } else if (perk_level(critter, PERK_DERMAL_IMPACT_ASSAULT_ENHANCEMENT)) {
                    value += 10;
                }
                break;
            case STAT_DAMAGE_RESISTANCE_LASER:
            case STAT_DAMAGE_RESISTANCE_FIRE:
            case STAT_DAMAGE_RESISTANCE_PLASMA:
                if (perk_level(critter, PERK_PHOENIX_ARMOR_IMPLANTS)) {
                    value += 5;
                } else if (perk_level(critter, PERK_PHOENIX_ASSAULT_ENHANCEMENT)) {
                    value += 10;
                }
                break;
            case STAT_RADIATION_RESISTANCE:
            case STAT_POISON_RESISTANCE:
                if (perk_level(critter, PERK_VAULT_CITY_INOCULATIONS)) {
                    value += 10;
                }
                break;
            }
        }

        value = min(max(value, gStatDescriptions[stat].minimumValue), gStatDescriptions[stat].maximumValue);
    } else {
        switch (stat) {
        case STAT_CURRENT_HIT_POINTS:
            value = critter_get_hits(critter);
            break;
        case STAT_CURRENT_POISON_LEVEL:
            value = critter_get_poison(critter);
            break;
        case STAT_CURRENT_RADIATION_LEVEL:
            value = critter_get_rads(critter);
            break;
        default:
            value = 0;
            break;
        }
    }

    return value;
}

// Returns base stat value (accounting for traits if critter is dude).
//
// 0x4AF3E0
int critterGetBaseStatWithTraitModifier(Object* critter, int stat)
{
    int value = critterGetBaseStat(critter, stat);

    if (critter == obj_dude) {
        value += traitGetStatModifier(stat);
    }

    return value;
}

// 0x4AF408
int critterGetBaseStat(Object* critter, int stat)
{
    Proto* proto;

    if (stat >= 0 && stat < SAVEABLE_STAT_COUNT) {
        proto_ptr(critter->pid, &proto);
        return proto->critter.data.baseStats[stat];
    } else {
        switch (stat) {
        case STAT_CURRENT_HIT_POINTS:
            return critter_get_hits(critter);
        case STAT_CURRENT_POISON_LEVEL:
            return critter_get_poison(critter);
        case STAT_CURRENT_RADIATION_LEVEL:
            return critter_get_rads(critter);
        }
    }

    return 0;
}

// 0x4AF474
int critterGetBonusStat(Object* critter, int stat)
{
    if (stat >= 0 && stat < SAVEABLE_STAT_COUNT) {
        Proto* proto;
        proto_ptr(critter->pid, &proto);
        return proto->critter.data.bonusStats[stat];
    }

    return 0;
}

// 0x4AF4BC
int critterSetBaseStat(Object* critter, int stat, int value)
{
    Proto* proto;

    if (!statIsValid(stat)) {
        return -5;
    }

    if (stat >= 0 && stat < SAVEABLE_STAT_COUNT) {
        if (stat > STAT_LUCK && stat <= STAT_POISON_RESISTANCE) {
            // Cannot change base value of derived stats.
            return -1;
        }

        if (critter == obj_dude) {
            value -= traitGetStatModifier(stat);
        }

        if (value < gStatDescriptions[stat].minimumValue) {
            return -2;
        }

        if (value > gStatDescriptions[stat].maximumValue) {
            return -3;
        }

        proto_ptr(critter->pid, &proto);
        proto->critter.data.baseStats[stat] = value;

        if (stat >= STAT_STRENGTH && stat <= STAT_LUCK) {
            critterUpdateDerivedStats(critter);
        }

        return 0;
    }

    switch (stat) {
    case STAT_CURRENT_HIT_POINTS:
        return critter_adjust_hits(critter, value - critter_get_hits(critter));
    case STAT_CURRENT_POISON_LEVEL:
        return critter_adjust_poison(critter, value - critter_get_poison(critter));
    case STAT_CURRENT_RADIATION_LEVEL:
        return critter_adjust_rads(critter, value - critter_get_rads(critter));
    }

    // Should be unreachable
    return 0;
}

// 0x4AF5D4
int critterIncBaseStat(Object* critter, int stat)
{
    int value = critterGetBaseStat(critter, stat);

    if (critter == obj_dude) {
        value += traitGetStatModifier(stat);
    }

    return critterSetBaseStat(critter, stat, value + 1);
}

// 0x4AF608
int critterDecBaseStat(Object* critter, int stat)
{
    int value = critterGetBaseStat(critter, stat);

    if (critter == obj_dude) {
        value += traitGetStatModifier(stat);
    }

    return critterSetBaseStat(critter, stat, value - 1);
}

// 0x4AF63C
int critterSetBonusStat(Object* critter, int stat, int value)
{
    if (!statIsValid(stat)) {
        return -5;
    }

    if (stat >= 0 && stat < SAVEABLE_STAT_COUNT) {
        Proto* proto;
        proto_ptr(critter->pid, &proto);
        proto->critter.data.bonusStats[stat] = value;

        if (stat >= STAT_STRENGTH && stat <= STAT_LUCK) {
            critterUpdateDerivedStats(critter);
        }

        return 0;
    } else {
        switch (stat) {
        case STAT_CURRENT_HIT_POINTS:
            return critter_adjust_hits(critter, value);
        case STAT_CURRENT_POISON_LEVEL:
            return critter_adjust_poison(critter, value);
        case STAT_CURRENT_RADIATION_LEVEL:
            return critter_adjust_rads(critter, value);
        }
    }

    // Should be unreachable
    return -1;
}

// 0x4AF6CC
void protoCritterDataResetStats(CritterProtoData* data)
{
    for (int stat = 0; stat < SAVEABLE_STAT_COUNT; stat++) {
        data->baseStats[stat] = gStatDescriptions[stat].defaultValue;
        data->bonusStats[stat] = 0;
    }
}

// 0x4AF6FC
void critterUpdateDerivedStats(Object* critter)
{
    int strength = critterGetStat(critter, STAT_STRENGTH);
    int perception = critterGetStat(critter, STAT_PERCEPTION);
    int endurance = critterGetStat(critter, STAT_ENDURANCE);
    int intelligence = critterGetStat(critter, STAT_INTELLIGENCE);
    int agility = critterGetStat(critter, STAT_AGILITY);
    int luck = critterGetStat(critter, STAT_LUCK);

    Proto* proto;
    proto_ptr(critter->pid, &proto);
    CritterProtoData* data = &(proto->critter.data);

    data->baseStats[STAT_MAXIMUM_HIT_POINTS] = critterGetBaseStatWithTraitModifier(critter, STAT_STRENGTH) + critterGetBaseStatWithTraitModifier(critter, STAT_ENDURANCE) * 2 + 15;
    data->baseStats[STAT_MAXIMUM_ACTION_POINTS] = agility / 2 + 5;
    data->baseStats[STAT_ARMOR_CLASS] = agility;
    data->baseStats[STAT_MELEE_DAMAGE] = max(strength - 5, 1);
    data->baseStats[STAT_CARRY_WEIGHT] = 25 * strength + 25;
    data->baseStats[STAT_SEQUENCE] = 2 * perception;
    data->baseStats[STAT_HEALING_RATE] = max(endurance / 3, 1);
    data->baseStats[STAT_CRITICAL_CHANCE] = luck;
    data->baseStats[STAT_BETTER_CRITICALS] = 0;
    data->baseStats[STAT_RADIATION_RESISTANCE] = 2 * endurance;
    data->baseStats[STAT_POISON_RESISTANCE] = 5 * endurance;
}

// 0x4AF854
char* statGetName(int stat)
{
    return statIsValid(stat) ? gStatDescriptions[stat].name : NULL;
}

// 0x4AF898
char* statGetDescription(int stat)
{
    return statIsValid(stat) ? gStatDescriptions[stat].description : NULL;
}

// 0x4AF8DC
char* statGetValueDescription(int value)
{
    if (value < PRIMARY_STAT_MIN) {
        value = PRIMARY_STAT_MIN;
    } else if (value > PRIMARY_STAT_MAX) {
        value = PRIMARY_STAT_MAX;
    }

    return gStatValueDescriptions[value - PRIMARY_STAT_MIN];
}

// 0x4AF8FC
int pcGetStat(int pcStat)
{
    return pcStatIsValid(pcStat) ? gPcStatValues[pcStat] : 0;
}

// 0x4AF910
int pcSetStat(int pcStat, int value)
{
    int result;

    if (!pcStatIsValid(pcStat)) {
        return -5;
    }

    if (value < gPcStatDescriptions[pcStat].minimumValue) {
        return -2;
    }

    if (value > gPcStatDescriptions[pcStat].maximumValue) {
        return -3;
    }

    if (pcStat != PC_STAT_EXPERIENCE || value >= gPcStatValues[PC_STAT_EXPERIENCE]) {
        gPcStatValues[pcStat] = value;
        if (pcStat == PC_STAT_EXPERIENCE) {
            result = pcAddExperienceWithOptions(0, true);
        } else {
            result = 0;
        }
    } else {
        result = pcSetExperience(value);
    }

    return result;
}

// Reset stats.
//
// 0x4AF980
void pcStatsReset()
{
    for (int pcStat = 0; pcStat < PC_STAT_COUNT; pcStat++) {
        gPcStatValues[pcStat] = gPcStatDescriptions[pcStat].defaultValue;
    }
}

// Returns experience to reach next level.
//
// 0x4AF9A0
int pcGetExperienceForNextLevel()
{
    return pcGetExperienceForLevel(gPcStatValues[PC_STAT_LEVEL] + 1);
}

// Returns exp to reach given level.
//
// 0x4AF9A8
int pcGetExperienceForLevel(int level)
{
    if (level >= PC_LEVEL_MAX) {
        return -1;
    }

    int v1 = level / 2;
    if ((level & 1) != 0) {
        return 1000 * v1 * level;
    } else {
        return 1000 * v1 * (level - 1);
    }
}

// 0x4AF9F4
char* pcStatGetName(int pcStat)
{
    return pcStat >= 0 && pcStat < PC_STAT_COUNT ? gPcStatDescriptions[pcStat].name : NULL;
}

// 0x4AFA14
char* pcStatGetDescription(int pcStat)
{
    return pcStat >= 0 && pcStat < PC_STAT_COUNT ? gPcStatDescriptions[pcStat].description : NULL;
}

// 0x4AFA34
int statGetFrmId(int stat)
{
    return statIsValid(stat) ? gStatDescriptions[stat].frmId : 0;
}

// Roll D10 against specified stat.
//
// This function is intended to be used with one of SPECIAL stats (which are
// capped at 10, hence d10), not with artitrary stat, but does not enforce it.
//
// An optional [modifier] can be supplied as a bonus (or penalty) to the stat's
// value.
//
// Upon return [howMuch] will be set to difference between stat's value
// (accounting for given [modifier]) and d10 roll, which can be positive (or
// zero) when roll succeeds, or negative when roll fails. Set [howMuch] to
// `NULL` if you're not interested in this value.
//
// 0x4AFA78
int statRoll(Object* critter, int stat, int modifier, int* howMuch)
{
    int value = critterGetStat(critter, stat) + modifier;
    int chance = roll_random(PRIMARY_STAT_MIN, PRIMARY_STAT_MAX);

    if (howMuch != NULL) {
        *howMuch = value - chance;
    }

    if (chance <= value) {
        return ROLL_SUCCESS;
    }

    return ROLL_FAILURE;
}

// 0x4AFAA8
int pcAddExperience(int xp)
{
    return pcAddExperienceWithOptions(xp, true);
}

// 0x4AFAB8
int pcAddExperienceWithOptions(int xp, bool a2)
{
    int newXp = gPcStatValues[PC_STAT_EXPERIENCE];
    newXp += xp;
    newXp += perk_level(obj_dude, PERK_SWIFT_LEARNER) * 5 * xp / 100;

    if (newXp < gPcStatDescriptions[PC_STAT_EXPERIENCE].minimumValue) {
        newXp = gPcStatDescriptions[PC_STAT_EXPERIENCE].minimumValue;
    }

    if (newXp > gPcStatDescriptions[PC_STAT_EXPERIENCE].maximumValue) {
        newXp = gPcStatDescriptions[PC_STAT_EXPERIENCE].maximumValue;
    }

    gPcStatValues[PC_STAT_EXPERIENCE] = newXp;

    while (gPcStatValues[PC_STAT_LEVEL] < PC_LEVEL_MAX) {
        if (newXp < pcGetExperienceForNextLevel()) {
            break;
        }

        if (pcSetStat(PC_STAT_LEVEL, gPcStatValues[PC_STAT_LEVEL] + 1) == 0) {
            int maxHpBefore = critterGetStat(obj_dude, STAT_MAXIMUM_HIT_POINTS);

            // You have gone up a level.
            MessageListItem messageListItem;
            messageListItem.num = 600;
            if (message_search(&gStatsMessageList, &messageListItem)) {
                display_print(messageListItem.text);
            }

            pc_flag_on(DUDE_STATE_LEVEL_UP_AVAILABLE);

            gsound_play_sfx_file("levelup");

            // NOTE: Uninline.
            int endurance = critterGetBaseStatWithTraitModifier(obj_dude, STAT_ENDURANCE);

            int hpPerLevel = endurance / 2 + 2;
            hpPerLevel += perk_level(obj_dude, PERK_LIFEGIVER) * 4;

            int bonusHp = critterGetBonusStat(obj_dude, STAT_MAXIMUM_HIT_POINTS);
            critterSetBonusStat(obj_dude, STAT_MAXIMUM_HIT_POINTS, bonusHp + hpPerLevel);

            int maxHpAfter = critterGetStat(obj_dude, STAT_MAXIMUM_HIT_POINTS);
            critter_adjust_hits(obj_dude, maxHpAfter - maxHpBefore);

            intface_update_hit_points(false);

            if (a2) {
                partyMemberIncLevels();
            }
        }
    }

    return 0;
}

// 0x4AFC38
int pcSetExperience(int xp)
{
    int oldLevel = gPcStatValues[PC_STAT_LEVEL];
    gPcStatValues[PC_STAT_EXPERIENCE] = xp;

    int level = 1;
    do {
        level += 1;
    } while (xp >= pcGetExperienceForLevel(level) && level < PC_LEVEL_MAX);

    int newLevel = level - 1;

    pcSetStat(PC_STAT_LEVEL, newLevel);
    pc_flag_off(DUDE_STATE_LEVEL_UP_AVAILABLE);

    // NOTE: Uninline.
    int endurance = critterGetBaseStatWithTraitModifier(obj_dude, STAT_ENDURANCE);

    int hpPerLevel = endurance / 2 + 2;
    hpPerLevel += perk_level(obj_dude, PERK_LIFEGIVER) * 4;

    int deltaHp = (oldLevel - newLevel) * hpPerLevel;
    critter_adjust_hits(obj_dude, -deltaHp);

    int bonusHp = critterGetBonusStat(obj_dude, STAT_MAXIMUM_HIT_POINTS);

    critterSetBonusStat(obj_dude, STAT_MAXIMUM_HIT_POINTS, bonusHp - deltaHp);

    intface_update_hit_points(false);

    return 0;
}