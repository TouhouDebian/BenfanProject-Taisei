/* Benfan Project: campus prototype. MIT; see COPYING.txt. */
#include "benfan.h"
#include "common_tasks.h"
#include "dialog/reimu.h"
#include "stage.h"
#include "util/graphics.h"

#define CHARACTER(cid, key, name, subtitle, art, bg) { \
    .id = cid, .lower_name = key, .asset_name = art, \
    .proper_name = name, .full_name = name, .title = subtitle, .menu_texture_name = bg, \
}
PlayerCharacter benfan_characters[4] = {
    CHARACTER(PLR_CHAR_REIMU, "international", "International", "Early departure / reverse commute", "reimu", "reimubg"),
    CHARACTER(PLR_CHAR_MARISA, "senior1", "Senior One", "Quick steps / learning the campus", "marisa", "marisa_bombbg"),
    CHARACTER(PLR_CHAR_YOUMU, "senior2", "Senior Two", "Balanced pace / reading the crowd", "youmu", "youmu_bombbg1"),
    CHARACTER(PLR_CHAR_SENIOR3, "senior3", "Senior Three", "Precise steps / familiar route", "reimu", "reimubg"),
};

static double benfan_property(Player *p, PlrProperty prop) {
    static const double fast[] = {5.6, 6.3, 5.9, 5.2};
    static const double slow[] = {2.2, 2.8, 2.5, 1.9};
    int id = p->mode->character->id;
    switch(prop) {
        case PLR_PROP_SPEED: return (p->inputflags & INFLAG_FOCUS) ? slow[id] : fast[id];
        case PLR_PROP_POC: return VIEWPORT_H / 3.5;
        case PLR_PROP_COLLECT_RADIUS: return (p->inputflags & INFLAG_FOCUS) ? 60 : 30;
        case PLR_PROP_BOMB_TIME: return 120 + id * 20;
        case PLR_PROP_DEATHBOMB_WINDOW: return 12;
    }
    UNREACHABLE;
}

TASK(benfan_controller, { BoxedPlayer player; }) {
    Player *p = TASK_BIND(ARGS.player);
    for(;;) {
        if(player_is_bomb_active(p)) {
            // Excuse me: open a local passage, without harming students.
            stage_clear_hazards_at(p->pos, 100 + 15 * p->mode->character->id, CLEAR_HAZARDS_ALL);
        }
        if(p->mode->shot_mode == PLR_SHOT_A && player_should_shoot(p) && global.frames % 5 == 0) {
            int id = p->mode->character->id;
            int count = id == 0 ? 3 : 2;
            for(int n = 0; n < count; ++n) {
                double dx = (n - (count - 1) * 0.5);
                PROJECTILE(
                    .proto = pp_rice, .type = PROJ_PLAYER,
                    .pos = p->pos + 10 * dx - 12 * I,
                    .move = move_linear((id == 0 ? dx * 1.8 : dx * 0.4) - 16 * I),
                    .color = RGB(0.3, 0.85, 1), .damage = id == 3 ? 150 : 110,
                    .damage_type = DMG_PLAYER_SHOT,
                );
            }
        }
        YIELD;
    }
}

static void player_rect(double x, double y, double w, double h, const Color *c) {
    r_color(c);
    r_mat_mv_push(); r_mat_mv_translate(x, y, 0); r_mat_mv_scale(w, h, 1);
    r_draw_quad(); r_mat_mv_pop();
}
static void benfan_draw_player(EntityInterface *ent) {
    Player *p = ENT_CAST(ent, Player);
    if(p->deathtime >= global.frames) return;
    if(!player_is_vulnerable(p) && (global.frames / 5) % 2) return;
    Color uniforms[] = {*RGB(.18, .58, .92), *RGB(.18, .22, .28), *RGB(.5, .56, .62), *RGB(.94, .94, .9)};
    double x = re(p->pos), y = im(p->pos);
    r_state_push(); r_shader_standard_notex();
    player_rect(x, y + 12, 22, 6, RGBA(0, 0, 0, .3));
    player_rect(x - 5, y + 8, 5, 9, RGB(.1, .15, .2));
    player_rect(x + 5, y + 8, 5, 9, RGB(.1, .15, .2));
    player_rect(x, y, 19, 15, &uniforms[p->mode->character->id]);
    player_rect(x, y - 11, 12, 10, RGB(.96, .81, .64));
    player_rect(x, y - 17, 13, 5, RGB(.1, .13, .16));
    player_rect(x, y, 7, 7, RGB(.1, .18, .23));
    player_rect(x, y, 3, 3, RGB(1, 1, 1));
    r_state_pop();
}
static void benfan_init(Player *p) {
    p->ent.draw_func = benfan_draw_player;
    INVOKE_TASK(benfan_controller, ENT_BOX(p));
}
#define MODE(index, shot, label, desc) { \
    .name = label, .description = desc, .spellcard_name = "Excuse Me - Open a Passage", \
    .character = &benfan_characters[index], .dialog = &dialog_tasks_reimu, .shot_mode = shot, \
    .procs = {.init = benfan_init, .property = benfan_property}, \
}
PlayerMode benfan_modes[8] = {
    MODE(0, PLR_SHOT_A, "Wide passage", "Shooting: spread signals clear the abstract crowd leader. Shift to walk; X to ask for passage. Upstream portraits are placeholders."),
    MODE(0, PLR_SHOT_B, "Early lunch", "Dodging: quiet outbound walk, crowded return. No shooting. Shift to walk; X opens a local passage."),
    MODE(1, PLR_SHOT_A, "Fast commute", "Shooting: fast movement and paired forward signals. Avoid the crowd and clear the pattern leader."),
    MODE(1, PLR_SHOT_B, "Quick steps", "Dodging: fastest movement; wider slow movement. Watch the rear warning before overtaking crowds arrive."),
    MODE(2, PLR_SHOT_A, "Steady stream", "Shooting: balanced movement and paired forward signals. Keep a clear line through merging lanes."),
    MODE(2, PLR_SHOT_B, "Read the lanes", "Dodging: balanced movement. Learn how adjacent groups leave staggered gaps."),
    MODE(3, PLR_SHOT_A, "Focused route", "Shooting: slower precision movement and stronger forward signals. A longer passage request helps at the bridge."),
    MODE(3, PLR_SHOT_B, "Familiar route", "Dodging: precise slow movement and the longest passage request. Stay calm in the return rush."),
};
