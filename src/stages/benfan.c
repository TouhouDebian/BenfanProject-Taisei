/* Benfan Project campus stages, MIT; see COPYING.txt. */
#include "benfan.h"
#include "common_tasks.h"
#include "projectile.h"
#include "util/graphics.h"
#include "resource/font.h"

static const char *patterns[] = {
    "Bell Sign - First Footsteps", "Companions - Side by Side", "Bridge Sign - Narrow Passage",
    "Queue Sign - Staggered Lines", "Dismissal - Three Streams", "Return Sign - Against the Tide", "Late Bell - Overtaken from Behind",
};
static const char *places_cn[] = {"一面 · 出楼", "二面 · 湖边", "三面 · 致远桥去程", "四面 · 食堂", "五面 · 三路汇流", "六面 · 致远桥逆流", "Extra · 晚去奔饭"};
static int warning_until;
static double warning_x;
static int section;
static int stage_index(void) { return clamp(global.stage->id - 1, 0, 6); }
static bool dodging(void) { return global.plr.mode->shot_mode == PLR_SHOT_B; }

static void block(double x, double y, double w, double h, const Color *color) {
    r_shader_standard_notex();
    r_color(color);
    r_mat_mv_push();
    r_mat_mv_translate(x, y, 0);
    r_mat_mv_scale(w, h, 1);
    r_draw_quad();
    r_mat_mv_pop();
}
static void student(double x, double y, const Color *uniform) {
    block(x, y + 9, 18, 7, RGBA(0, 0, 0, .23));
    block(x - 4, y + 7, 4, 8, RGB(.13, .18, .23));
    block(x + 4, y + 7, 4, 8, RGB(.13, .18, .23));
    block(x, y, 15, 13, uniform);
    block(x, y - 10, 10, 9, RGB(.95, .8, .64));
    block(x, y - 15, 11, 4, RGB(.12, .14, .18));
    block(x, y, 5, 3, RGB(1, 1, 1)); // small, visible collision reference
}
static void draw_walker(Projectile *p, int t, ProjDrawRuleArgs args) {
    r_state_push();
    student(re(p->pos), im(p->pos), &p->color);
    r_state_pop();
}
static void walker(cmplx pos, cmplx velocity, int color) {
    Color colors[] = { *RGB(.25, .58, .84), *RGB(.25, .3, .38), *RGB(.55, .6, .64), *RGB(.88, .9, .94) };
    PROJECTILE(
        .sprite = "ball", .max_viewport_dist = 64, .pos = pos, .move = move_linear(velocity),
        .color = &colors[color % 4], .size = 20 + 30 * I, .collision_size = 7 + 9 * I,
        .draw_rule = { .func = draw_walker },
        .flags = PFLAG_NOSPAWNEFFECTS | PFLAG_NOCOLLISIONEFFECT | PFLAG_NOCLEAREFFECT,
    );
}
TASK(rear_group, { double x; double speed; int count; int color; }) {
    // Visible warning precedes the arrival; there is never a spawn at the player's feet.
    warning_x = ARGS.x;
    warning_until = global.frames + 90;
    WAIT(90);
    for(int j = 0; j < ARGS.count; ++j) {
        walker(ARGS.x + 25 * j + (VIEWPORT_H + 24) * I, -ARGS.speed * I, ARGS.color);
    }
}
static void crowd_wave(int wave, bool climax) {
    int k = stage_index();
    int id = global.plr.mode->character->id;
    double speed = difficulty_value(1.35, 1.7, 2.05, 2.4) + .10 * k;
    double gap = 58 + 32 * (1 + sin(wave * .71 + k));
    double gap_x = 70 + (VIEWPORT_W - 140) * (.5 + .42 * sin(wave * .43 + id));
    bool early = id == 0 && k < 3 && !climax;
    if(early && wave % 3 != 0) return;
    if(k == 3 && !climax && wave % 2) return;
    for(int lane = 0; lane < 10; ++lane) {
        double x = 24 + lane * (VIEWPORT_W - 48) / 9;
        if(fabs(x - gap_x) < gap) continue;
        if(early && lane % 3) continue;
        // Three source streams merge diagonally; other sections use fixed travel lanes.
        double dx = k == 4 ? (VIEWPORT_W * .5 - x) * .0018 : .15 * sin(wave + lane);
        walker(x - 22 * I, dx + speed * I, (lane + wave + id) % 4);
    }
    if((k == 6 || (id > 0 && k < 3)) && wave % 4 == 1) {
        double x = 35 + (wave * 73 % (int)(VIEWPORT_W - 110));
        INVOKE_TASK(rear_group, x, speed + .7, k == 6 ? 3 : 2, id);
    }
}
TASK_WITH_INTERFACE(benfan_spell, BossAttack) {
    Boss *b = INIT_BOSS_ATTACK(&ARGS);
    b->move = move_from_towards(b->pos, VIEWPORT_W * .5 + 95 * I, .04);
    BEGIN_BOSS_ATTACK(&ARGS);
    for(int wave = 0;; ++wave) {
        crowd_wave(wave, true);
        WAIT(difficulty_value(72, 60, 52, 44));
    }
}
TASK(benfan_timeline) {
    int k = stage_index();
    section = 0;
    WAIT(180);
    int waves = k == 6 ? 22 : 12 + k * 2;
    for(int wave = 0; wave < waves; ++wave) {
        crowd_wave(wave, false);
        WAIT(difficulty_value(88, 74, 64, 56));
    }
    WAIT(180);
    stage_clear_hazards(CLEAR_HAZARDS_ALL);
    section = 1;
    if(dodging()) {
        // A timed crowd challenge, with no target to attack.
        int duration = k == 6 ? 36 : 18 + 2 * k;
        for(int wave = 0; wave * 60 < duration * 60; ++wave) {
            crowd_wave(wave, true);
            WAIT(60);
        }
    } else {
        Boss *b = create_boss("Crowd Conductor (placeholder)", "cirno", VIEWPORT_W * .5 + 80 * I);
        global.boss = b;
        for(int n = 0; n < (k == 6 ? 3 : 2); ++n) {
            boss_add_attack_task(b, AT_Spellcard, patterns[k], 22 + k * 2, 6500 + k * 1200,
                TASK_INDIRECT(BossAttack, benfan_spell), NULL);
        }
        boss_engage(b);
        WAIT_EVENT(&b->events.defeated);
    }
    stage_clear_hazards(CLEAR_HAZARDS_ALL);
    section = 2;
    WAIT(120);
    stage_finish(GAMEOVER_SCORESCREEN);
}
static void begin(void) {
    section = 0;
    warning_until = 0;
    stage_set_voltage_thresholds(0, 0, 0, 0);
    stage_start_bgm("stage1");
    INVOKE_TASK(benfan_timeline);
}
static void preload(ResourceGroup *rg) {
    res_group_preload(rg, RES_BGM, RESF_OPTIONAL, "stage1", NULL);
    res_group_preload(rg, RES_ANIM, RESF_DEFAULT, "boss/cirno", NULL);
    res_group_preload(rg, RES_FONT, RESF_DEFAULT, "standard_cjk", NULL);
}
static void draw(void) {
    int k = stage_index();
    double scroll = fmod(global.frames * (section == 2 ? .3 : 1.0), 96);
    bool bridge = k == 2 || k == 5 || k == 6;
    r_state_push();
    set_ortho(VIEWPORT_W, VIEWPORT_H);
    r_disable(RCAP_DEPTH_TEST);
    r_disable(RCAP_CULL_FACE);
    block(VIEWPORT_W / 2., VIEWPORT_H / 2., VIEWPORT_W, VIEWPORT_H, RGB(.12, .29, .25));
    if(bridge) block(VIEWPORT_W / 2., VIEWPORT_H / 2., VIEWPORT_W, VIEWPORT_H, RGB(.15, .34, .43));
    block(VIEWPORT_W / 2., VIEWPORT_H / 2., VIEWPORT_W - 48, VIEWPORT_H, RGB(.72, .71, .65));
    for(int y = -96; y < VIEWPORT_H + 96; y += 96) {
        block(VIEWPORT_W / 2., y + scroll, VIEWPORT_W - 52, 2, RGB(.58, .58, .54));
        if(bridge) {
            block(19, y + scroll, 9, 56, RGB(.85, .78, .65));
            block(VIEWPORT_W - 19, y + scroll, 9, 56, RGB(.85, .78, .65));
        } else {
            block(10, y + scroll, 16, 40, RGB(.17, .4, .28));
            block(VIEWPORT_W - 10, y + scroll, 16, 40, RGB(.17, .4, .28));
        }
    }
    if(k == 0 || k == 3) {
        block(55, 140 + scroll, 50, 110, k == 3 ? RGB(.82, .65, .28) : RGB(.48, .61, .72));
        for(int y = 100; y < 190; y += 22) block(55, y + scroll, 35, 8, RGB(.2, .3, .35));
    }
    text_draw(places_cn[k], &(TextParams){ .shader = "text_default", .font = "standard_cjk", .pos = {14, 25}, .color = RGB(.1, .15, .18) });
    text_draw(dodging() ? "DODGING / X: excuse me" : "SHOOTING / Z: fire / X: passage", &(TextParams){ .shader = "text_default",
        .pos = {14, 49}, .color = RGB(.1, .15, .18), .font = "small" });
    if(section == 1) text_draw(patterns[k], &(TextParams){ .shader = "text_default", .pos = {14, 73}, .font = "small", .color = RGB(.16, .2, .24) });
    if(section == 2) text_draw(k == 3 ? "Lunch finished - time to return" : "Passage cleared", &(TextParams){ .shader = "text_default",
        .pos = {VIEWPORT_W / 2., VIEWPORT_H / 2.}, .align = ALIGN_CENTER, .color = RGB(1, 1, 1) });
    if(global.frames < warning_until) {
        block(warning_x + 25, VIEWPORT_H - 26, 100, 46, RGBA(.7, .22, .08, .9));
        text_draw("^^ BEHIND ^^", &(TextParams){ .shader = "text_default", .pos = {warning_x + 25, VIEWPORT_H - 20},
            .font = "small", .align = ALIGN_CENTER, .color = RGB(1, 1, 1) });
    }
    r_state_pop();
}
StageProcs benfan_stage_procs = { .begin = begin, .preload = preload, .draw = draw, .shader_rules = (ShaderRule[]){NULL} };
