#include <stdio.h>
#include <bangtal.h>
#include <math.h>
#include <stdlib.h>
#define STUCK_SPEED 4
#define ABLEJUMP 1
#define SPEED_X 7
#define POWER_JUMP 22
#define GRAVITY 2
#define STAGE_NUM 12
#define numWall 25
#define MONSTER_NUM 5
#define SIZEOFFIREBALL 20
#define BULLETSPEED 20
#define SCRIPT_NUM 64

ObjectID script[SCRIPT_NUM]; //스크립트 배경
SceneID back[STAGE_NUM]; //배경
SceneID main_scene;
SceneID game_over_back;

int scene_num = 0;
int temp = 0;
int script_num = 0;

bool activated = 1;

//클리어 포탈 위치
int portal_x[STAGE_NUM] = { 1100, 1200, 1200, 1200, 1200, 1150,1200, 150,50, 1100, 1200 };
int portal_y[STAGE_NUM] = { 20, 20, 20, 20, 20, 420, 20, 75, 320, 40, 20 };

//플레이어 스폰 위치
int spawn_x[STAGE_NUM] = { 0 , 0 , 0,  0, 0, 0, 0, 0 , 0, 0 , 600 };
int spawn_y[STAGE_NUM] = { 20, 20, 20, 20, 20, 20, 20, 420, 20, 20, 500 };

/*
int portal_x[STAGE_NUM] = { 1100, 1200, 1200, 1200, 1200, 1150,1200, 1200, 50, 1100, 1200 };
int portal_y[STAGE_NUM] = { 20, 20, 20, 20, 20, 420, 20, 20, 320, 40, 20 };

//플레이어 스폰 위치
int spawn_x[STAGE_NUM] = { 0 , 0 , 0, 0 , 0, 0 , 0, 0 , 0, 0 , 600 };
int spawn_y[STAGE_NUM] = { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 600 };*/

//클리어 포탈 크기(50, 100)
//클리어 포탈보다 palyer 크기가 작으면 오류 남.(정상작동 x)
int portal_xtox = 100;
int portal_ytoy = 50;
ObjectID portal[STAGE_NUM];


//메인화면 기본 버튼
ObjectID start_button;
ObjectID end_button;

struct wall   //벽 구조체
{
    ObjectID obj;
    int x, y;       //좌표
    int xtox, ytoy; //너비, 높이

};

struct player       //플레이어 구조체
{
    ObjectID obj;
    int x, y;
    int xtox, ytoy;
    int speed_x = 0;  //
    int speed_y = 0;
    int jumpState = 0;   //점프상태

    bool look = 1;        //보는 방향
};

struct mob      //몬스터 구조체
{
    ObjectID obj;
    int x, y;
    int xtox, ytoy;
    int speed = 0;
    int move_range = 0;
    int stage = 0;
    int look = 0;
    int walk_state = 0;
    int move_now_cnt = 0;
    int move_max_cnt = 0;
    int activated = 0;

};

struct fireball //투사체 구조체
{
    ObjectID obj;
    int x, y;
    int vector;

    int endX;

    bool activated = 0;
};


TimerID frame;
//걷는 모션 속도
TimerID walk_timer;
TimerID walk_monster;
//사자 맵 제한시간
TimerID lion_timer;

bool stuck = 0;           //벽타기 중인가


//사자 제한시간


//벽개수 구조체
wall wall1[numWall];

mob monster[MONSTER_NUM];

//스테이지 별 벽 개수
int stage_by_wall[STAGE_NUM] = { 4, 12, 14, 12, 4, 9, 18, 20, 8, 14, 13 };

//스테이지 별 몹 개수
int stage_by_mob[STAGE_NUM] = { 0, 0, 0, 0, 1, 2, 2, 4, 5, 4, 3 };

//걷는 상태
int walk_state = 1;
int player_vec = 0;


//게임 재시작에 사용
bool input_key = true;
bool shot_fireball = false;
ObjectID restart_button;

player player1;

//파이어볼
fireball bullet[5];
int ableFireball = 5;


bool Rkey = 0;
bool Lkey = 0;

int checkPlayer_AllWallToMoveX(player* obj1, wall obj2[], int obj1Vector);
void move_x(player* agent, int stage);
int checkPlayer_AllWallToMoveY(player* obj1, wall obj2[], int obj1Vector);
void move_y(player* agent, int stage);
void makeWall();
void game_init();
void make_back();
void make_script();
void check_clear(int stage);
void next_stage();
void spawn_monster(mob* monster, const char* c, int x, int y, int xtox, int ytoy, int speed, int move_range, int stage, int look, bool show_mob, bool activated);
void make_monster();
void check_hit(mob* monster, int stage);
void move_monster(mob* monster, int stage);
void change_monster_img(mob* monster, int stage);
void game_over();
void make_button();
void reset();
void shot(player* agent);
void moveBullet();
void player_walk();
void out_of_map();
void next_stage2();
void next_script();
ObjectID createObject(const char* image, SceneID scene, int x, int y, bool shown);


void timerCallback(TimerID timer)
{
    if (activated == false)
    {
        setTimer(timer, 0.01f);
        startTimer(timer);
        return;
    }
    player1.speed_y -= GRAVITY; //중력 가속도
    //printf("\nsped : %d y : %d", player1.speed_y, player1.y);
    if (input_key) {
        move_x(&player1, scene_num);
        move_y(&player1, scene_num);
        setTimer(timer, 0.01f);
        startTimer(timer);
    }
    //printf("\nsped : %d y : %d", player1.speed_y, player1.y);
    //printf("\n%d", player1.look);


    //포탈에 도달했는지
    check_clear(scene_num);

    //총알
    if (shot_fireball) moveBullet();

    //맵밖으로 나갔는지
    out_of_map();

    //몬스터 움직이고 몹에게 맞았는지
    for (int i = 0; i < stage_by_mob[scene_num]; i++) {
        //printf("\nDone %d", i);
        if (monster[i].activated == 0) continue;
        move_monster(&monster[i], scene_num);
        check_hit(&monster[i], scene_num);

    }

    if (timer == walk_timer) {
        player_walk();
        setTimer(timer, 0.15f);
        startTimer(timer);
    }

    if (timer == walk_monster) {
        for (int i = 0; i < MONSTER_NUM; i++) {
            change_monster_img(&monster[i], scene_num);
            setTimer(timer, 0.1f);
            startTimer(timer);
        }
    }

    if (timer == lion_timer) {
        game_over();
    }

}

void keyboardCallback(KeyCode c, KeyState s)
{
    if (c == KeyCode::KEY_UP_ARROW && s == KeyState::KEY_PRESSED)
    {

        if (player1.jumpState < ABLEJUMP)
        {
            stuck = false;   //벽에서 떨어짐
            player1.jumpState++;   //점프 갯수 ++
            player1.speed_y = POWER_JUMP;    //위로 가속도
        }

    }
    if (c == KeyCode::KEY_RIGHT_ARROW)
    {
        if (s == KeyState::KEY_PRESSED)
        {

            stuck = false; //벽에서 떨어짐
            Rkey = 1;       //
            player1.speed_x = SPEED_X;
            player1.look = 1;
        }
        else
        {
            stuck = false;
            Rkey = 0;
            if (Lkey)
            {
                player1.speed_x = -SPEED_X;
                player1.look = 0;
            }
            else player1.speed_x = 0;
        }


    }

    if (c == KeyCode::KEY_LEFT_ARROW)
    {
        if (s == KeyState::KEY_PRESSED)
        {
            Lkey = 1;
            player1.speed_x = -SPEED_X;
            player1.look = 0;
        }
        else
        {
            Lkey = 0;
            if (Rkey)
            {
                player1.speed_x = SPEED_X;
                player1.look = 1;
            }
            else player1.speed_x = 0;
        }


    }


    if (c == KeyCode::KEY_SPACE && s == KeyState::KEY_PRESSED)
    {

        if (shot_fireball) shot(&player1);

    }

}

void mouseCallback(ObjectID object, int x, int y, MouseAction action) {
    if (object == restart_button) {
        reset();
    }

    if (object == start_button) {
        showObject(script[0]);
        hideObject(start_button);
        hideObject(end_button);
 
    }
    else if (object == end_button) {
        endGame();
    }


    if (object == script[0]) {
        hideObject(script[0]);
        showObject(script[1]);
    }
    //Script 1
    else if (object == script[1]) {
        hideObject(script[1]);
        showObject(script[2]);
    }

    else if (object == script[2]) {
        hideObject(script[2]);
        showObject(script[3]);
    }

    else if (object == script[3]) {
        hideObject(script[3]);
        showObject(script[4]);
    }

    else if (object == script[4]) {
        hideObject(script[4]);
        showObject(script[5]);
    }

    else if (object == script[5]) {
        hideObject(script[5]);
        showObject(script[6]);
    }

    //장면바뀜
    else if (object == script[6]) {
        hideObject(script[6]);
        printf("%d", script_num);
        enterScene(back[0]);
    }

    else if (object == script[7]) {
        hideObject(script[7]);
        showObject(script[8]);
    }

    //장면전환
    else if (object == script[8]) {
        //next_script();
        hideObject(script[8]);
        //script_num++;
        next_stage2();
    }

    else if (object == script[9]) {
        hideObject(script[9]);
        showObject(script[10]);
    }

    else if (object == script[10]) {
        hideObject(script[10]);
        showObject(script[11]);
    }
    else if (object == script[11]) {
        hideObject(script[11]);
        showObject(script[12]);
    }
    else if (object == script[12]) {
        hideObject(script[12]);
        showObject(script[13]);
    }
    else if (object == script[13]) {
        hideObject(script[13]);
        showObject(script[14]);
    }
    else if (object == script[14]) {
        hideObject(script[14]);
        next_stage2();
    }

    else if (object == script[15]) {
        hideObject(script[15]);
        showObject(script[16]);
    }

    else if (object == script[16]) {
        hideObject(script[16]);
        showObject(script[17]);
    }

    else if (object == script[17]) {
        hideObject(script[17]);
        showObject(script[18]);
    }

    else if (object == script[18]) {
        hideObject(script[18]);
        next_stage2();
    }

    else if (object == script[19]) {
        hideObject(script[19]);
        showObject(script[20]);
    }

    else if (object == script[20]) {
    hideObject(script[20]);
    showObject(script[21]);
    }

    else if (object == script[21]) {
    hideObject(script[21]);
    showObject(script[22]);
    }

    else if (object == script[22]) {
    hideObject(script[22]);
    showObject(script[23]);
    }

    else if (object == script[23]) {
    hideObject(script[23]);
    showObject(script[24]);
    }

    else if (object == script[24]) {
    hideObject(script[24]);
    showObject(script[25]);
    }

    else if (object == script[25]) {
    hideObject(script[25]);
    showObject(script[26]);
    }

    else if (object == script[26]) {
    hideObject(script[26]);
    showObject(script[27]);
    }

    else if (object == script[27]) {
    hideObject(script[27]);
    showObject(script[28]);
    }

    else if (object == script[28]) {
    hideObject(script[28]);
    showObject(script[29]);
    }

    else if (object == script[29]) {
        hideObject(script[29]);
        showObject(script[30]);
    }

    else if (object == script[30]) {
    hideObject(script[30]);
    showObject(script[31]);
    }

    else if (object == script[31]) {
    hideObject(script[31]);
    showObject(script[32]);
    }

    else if (object == script[32]) {
        hideObject(script[32]);
        showObject(script[33]);
    }

    else if (object == script[33]) {
    hideObject(script[33]);
    showObject(script[34]);
    }

    else if (object == script[34]) {
        hideObject(script[34]);
        showObject(script[35]);
    }

    else if (object == script[35]) {
    hideObject(script[35]);
    showObject(script[36]);
    }

    else if (object == script[36]) {
    hideObject(script[36]);
    showObject(script[37]);
    }

    else if (object == script[37]) {
    hideObject(script[37]);
    showObject(script[38]);
    }

    else if (object == script[38]) {
    hideObject(script[38]);
    showObject(script[39]);
    }

    else if (object == script[39]) {
    hideObject(script[39]);
    showObject(script[40]);
    }

    else if (object == script[40]) {
    hideObject(script[40]);
    showObject(script[41]);
    }

    else if (object == script[41]) {
    hideObject(script[41]);
    showObject(script[42]);
    }

    else if (object == script[42]) {
    hideObject(script[42]);
    showObject(script[43]);
    }

    else if (object == script[43]) {
    hideObject(script[43]);
    showObject(script[44]);
    }

    else if (object == script[44]) {
    hideObject(script[44]);
    showObject(script[45]);
    }
 
    else if (object == script[45]) {
    hideObject(script[45]);
    showObject(script[46]);
    }

    else if (object == script[46]) {
    hideObject(script[46]);
    showObject(script[47]);
    }

    else if (object == script[47]) {
    hideObject(script[47]);
    next_stage2();
    }

    else if (object == script[48]) {
    hideObject(script[48]);
    showObject(script[49]);
    }

    else if (object == script[49]) {
    hideObject(script[49]);
    showObject(script[50]);
    }

    else if (object == script[50]) {
    hideObject(script[50]);
    next_stage2();
    }

    else if (object == script[51]) {
    hideObject(script[51]);
    showObject(script[52]);
    }

    else if (object == script[52]) {
    hideObject(script[52]);
    showObject(script[53]);
    }

    else if (object == script[53]) {
    hideObject(script[53]);
    showObject(script[54]);
    }

    else if (object == script[54]) {
    hideObject(script[54]);
    showObject(script[55]);
    }

    else if (object == script[55]) {
    hideObject(script[55]);
    showObject(script[56]);
    }

    else if (object == script[56]) {
    hideObject(script[56]);
    showObject(script[57]);
    }

    else if (object == script[57]) {
    hideObject(script[57]);
    showObject(script[58]);
    }

    else if (object == script[58]) {
    hideObject(script[58]);
    showObject(script[59]);
    }

    else if (object == script[59]) {
    hideObject(script[59]);
    showObject(script[60]);
    }

    else if (object == script[60]) {
    hideObject(script[60]);
    showObject(script[61]);
    }

    else if (object == script[61]) {
    hideObject(script[61]);
    showObject(script[62]);
    }

    else if (object == script[62]) {
    hideObject(script[62]);
    showObject(script[63]);
    }

    else if (object == script[63]) {
    endGame();
    }


    //Script END
}

ObjectID createObject(const char* image, SceneID scene, int x, int y, bool shown) {

    ObjectID object = createObject(image);

    locateObject(object, scene, x, y);
    if (shown) {
        showObject(object);
    }
    return object;
}

void out_of_map() {
    if (player1.y < 0) {
        game_over();
    }
}

void reset() {
    input_key = true;


    enterScene(back[scene_num]);
    locateObject(player1.obj, back[scene_num], player1.x = spawn_x[scene_num], player1.y = spawn_y[scene_num]);
    hideObject(restart_button);
    make_monster();

    setTimer(frame, 0.01f);
    startTimer(frame);

    if (4 <= scene_num && scene_num <= 7) {
        showTimer(lion_timer);
        setTimer(lion_timer, 30.f);
        startTimer(lion_timer);
    }
    if (scene_num == 8)
    {
        showMessage("스페이스바를 눌러 공격하자");
    }

}

//버튼 만드는 함수, show는 안해줌
void make_button() {
    restart_button = createObject("restart_button.png");
    start_button = createObject("start_button.png", main_scene, 505, 100, true);
    end_button = createObject("end_button.png", main_scene, 505, 30, true);
}

//배경
void make_back() {
    main_scene = createScene("메인화면", "main.png");
    back[0] = createScene("Stage_0", "tutorial.png");
    back[1] = createScene("Stage_1", "Stage_1.png");
    back[2] = createScene("Stage_2-1", "Stage_2.png");
    back[3] = createScene("Stage_2-2", "Stage_2.png");
    back[4] = createScene("Stage_3-1", "Stage_3.png");
    back[5] = createScene("Stage_3-2", "Stage_3.png");
    back[6] = createScene("Stage_3-3", "Stage_3.png");
    back[7] = createScene("Stage_3-4", "Stage_3.png");
    back[8] = createScene("Stage_4", "Fback.png");
    back[9] = createScene("Stage_5-1", "Fback.png");
    back[10] = createScene("Stage_5-2", "Fback.png");
    game_over_back = createScene("게임오버", "game_over_back.png");
}

//스크립트 배경(오브젝트로 생성)
void make_script() {
    //튜토리얼 전
    script[0] = createObject("Scene/Scene_1.png", main_scene, -73, 0, false);
    script[1] = createObject("Scene/Scene_2.png", main_scene, -73, 0, false);
    script[2] = createObject("Scene/Scene_3.png", main_scene, -73, 0, false);
    script[3] = createObject("Scene/Scene_4.png", main_scene, -73, 0, false);
    script[4] = createObject("Scene/Scene_5.png", main_scene, -73, 0, false);
    script[5] = createObject("Scene/Scene_6.png", main_scene, -73, 0, false);
    script[6] = createObject("Scene/Scene_7.png", main_scene, -73, 0, false);

    //허수아비 전
    script[7] = createObject("Scene/Scene_8.png", back[0], 0, 0, false);
    script[8] = createObject("Scene/Scene_9.png", back[0], 0, 0, false);

    //나무꾼 전
    script[9] = createObject("Scene/Scene_10.png", back[1], 0, 0, false);
    script[10] = createObject("Scene/Scene_11.png", back[1], 0, 0, false);
    script[11] = createObject("Scene/Scene_12.png", back[1], 0, 0, false);
    script[12] = createObject("Scene/Scene_13.png", back[1], 0, 0, false);
    script[13] = createObject("Scene/Scene_14.png", back[1], 0, 0, false);
    script[14] = createObject("Scene/Scene_15.png", back[1], 0, 0, false);

    //사자 전
    script[15] = createObject("Scene/Scene_16.png", back[3], 0, 0, false);
    script[16] = createObject("Scene/Scene_17.png", back[3], 0, 0, false);
    script[17] = createObject("Scene/Scene_18.png", back[3], 0, 0, false);
    script[18] = createObject("Scene/Scene_19.png", back[3], 0, 0, false);

    //화염구 전
    script[19] = createObject("Scene/Scene_20.png", back[7], 0, 0, false);
    script[20] = createObject("Scene/Scene_21.png", back[7], 0, 0, false);
    script[21] = createObject("Scene/Scene_22.png", back[7], 0, 0, false);
    script[22] = createObject("Scene/Scene_23.png", back[7], 0, 0, false);
    script[23] = createObject("Scene/Scene_24.png", back[7], 0, 0, false);
    script[24] = createObject("Scene/Scene_25.png", back[7], 0, 0, false);
    script[25] = createObject("Scene/Scene_26.png", back[7], 0, 0, false);
    script[26] = createObject("Scene/Scene_27.png", back[7], 0, 0, false);
    script[27] = createObject("Scene/Scene_28.png", back[7], 0, 0, false);
    script[28] = createObject("Scene/Scene_29.png", back[7], 0, 0, false);
    script[29] = createObject("Scene/Scene_30.png", back[7], 0, 0, false);
    script[30] = createObject("Scene/Scene_31.png", back[7], 0, 0, false);
    script[31] = createObject("Scene/Scene_32.png", back[7], 0, 0, false);
    script[32] = createObject("Scene/Scene_33.png", back[7], 0, 0, false);
    script[33] = createObject("Scene/Scene_34.png", back[7], 0, 0, false);
    script[34] = createObject("Scene/Scene_35.png", back[7], 0, 0, false);
    script[35] = createObject("Scene/Scene_36.png", back[7], 0, 0, false);
    script[36] = createObject("Scene/Scene_37.png", back[7], 0, 0, false);
    script[37] = createObject("Scene/Scene_38.png", back[7], 0, 0, false);
    script[38] = createObject("Scene/Scene_39.png", back[7], 0, 0, false);
    script[39] = createObject("Scene/Scene_40.png", back[7], 0, 0, false);
    script[40] = createObject("Scene/Scene_41.png", back[7], 0, 0, false);
    script[41] = createObject("Scene/Scene_42.png", back[7], 0, 0, false);
    script[42] = createObject("Scene/Scene_43.png", back[7], 0, 0, false);
    script[43] = createObject("Scene/Scene_44.png", back[7], 0, 0, false);
    script[44] = createObject("Scene/Scene_45.png", back[7], 0, 0, false);
    script[45] = createObject("Scene/Scene_46.png", back[7], 0, 0, false);
    script[46] = createObject("Scene/Scene_47.png", back[7], 0, 0, false);
    script[47] = createObject("Scene/Scene_48.png", back[7], 0, 0, false);

    script[48] = createObject("Scene/Scene_49.png", back[8], 0, 0, false);
    script[49] = createObject("Scene/Scene_50.png", back[8], 0, 0, false);
    script[50] = createObject("Scene/Scene_51.png", back[8], 0, 0, false);

    script[51] = createObject("Scene/Scene_52.png", back[10], 0, 0, false);
    script[52] = createObject("Scene/Scene_53.png", back[10], 0, 0, false);
    script[53] = createObject("Scene/Scene_54.png", back[10], 0, 0, false);
    script[54] = createObject("Scene/Scene_55.png", back[10], 0, 0, false);
    script[55] = createObject("Scene/Scene_56.png", back[10], 0, 0, false);
    script[56] = createObject("Scene/Scene_57.png", back[10], 0, 0, false);
    script[57] = createObject("Scene/Scene_58.png", back[10], 0, 0, false);
    script[58] = createObject("Scene/Scene_59.png", back[10], 0, 0, false);
    script[59] = createObject("Scene/Scene_60.png", back[10], 0, 0, false);
    script[60] = createObject("Scene/Scene_61.png", back[10], 0, 0, false);
    script[61] = createObject("Scene/Scene_62.png", back[10], 0, 0, false);
    script[62] = createObject("Scene/Scene_63.png", back[10], 0, 0, false);
    script[63] = createObject("Scene/Scene_END.png", back[10], 0, 0, false);
;




    //Stage 5-1
    //Stage 5-2

  
}

//포탈 locate
void make_portal() {
    for (int i = 0; i < STAGE_NUM; i++) {
        portal[i] = createObject("portal.png");
        locateObject(portal[i], back[i], portal_x[i], portal_y[i]);
        showObject(portal[i]);
        if (i == 1) {
            setObjectImage(portal[i], "scarecrow_portal.png");
            continue;
        }
        else if (i == 2) {
            setObjectImage(portal[i], "lodge_portal.png");
            continue;
        }
        else if (i == 3) {
            setObjectImage(portal[i], "oil_portal.png");
            continue;
        }
        else if (i == 7) {
            setObjectImage(portal[i], "lion_portal.png");
            continue;
        }
    }
}

//이전 object 제거, 현재 스테이지 object생성, scene_num++
void next_stage() {
    //hide_prestage_obj(scene_num);
    if (temp == 1) return;
    activated = 0;
    
    for (int i = 0; i < 5; i++) {
        hideObject(bullet[i].obj);
        bullet[i].activated = 0;
    }
    for (int i = 0; i < stage_by_mob[scene_num]; i++)
    {
        hideObject(monster[i].obj);
        printf("\ndel");
    }
    for (int i = 0; i < stage_by_wall[scene_num]; i++)
    {
        hideObject(wall1[i].obj);

    }
    hideObject(portal[scene_num]);
    hideObject(player1.obj);
    
    if (scene_num == 0) {
        showObject(script[7]);
    }

    else if (scene_num == 1) {
        showObject(script[9]);
    }

    else if (scene_num == 3) {
        showObject(script[15]);
    }

    else if (scene_num == 7) {
        stopTimer(lion_timer);
        hideTimer();
        
        showObject(script[19]);
    }

    else if (scene_num == 8) {
        showObject(script[48]);
    }

    else if (scene_num == 10) {
        showObject(script[51]);
    }


}

void next_stage2()
{
    printf("%d", scene_num);
    if (scene_num == 0 || scene_num == 1 || scene_num == 3 || scene_num == 7 || scene_num == 8 || scene_num == 10) {
        printf("!!");
        next_stage();
        if (temp == 0) {
            temp = 1;
            return;
        }
    }
    activated = 1;
    temp = 0;

    //printf("asdfasdfasdfadsffasfsa");
    scene_num++;
    //scene_num += 4;
    makeWall();
    make_monster();
    enterScene(back[scene_num]);
    locateObject(player1.obj, back[scene_num], player1.x = spawn_x[scene_num], player1.y = spawn_y[scene_num]);
    showObject(player1.obj);
    //show_nowstage_obj(scene_num);

    for (int i = 0; i < stage_by_wall[scene_num]; i++)
    {
        showObject(wall1[i].obj);

    }

    if (4 <= scene_num && scene_num <= 7) {
        setTimer(lion_timer, 30.f);
        startTimer(lion_timer);
        showTimer(lion_timer);
    }

    if (scene_num == 7) {
        portal_xtox = 100;
        portal_ytoy = 50;
    }

    else if (scene_num == 8) {
        
        portal_xtox = 50;
        portal_ytoy = 100;
    }

    if (scene_num >= 8) {
        shot_fireball = true;
    }

    if (scene_num == 5) {
        showMessage("30초 안에 사자를 찾자!");
    }
    
    if (scene_num == 8) {
        showMessage("스페이스바를 눌러 공격하자");
    }

}

//wall 구조체, img 파일명, x, y, 너비, 높이, 속도, 이동 범위 ,배경 stage, 보는 방향, bool showimg
void spawn_monster(mob* monster, const char* c, int x, int y, int xtox, int ytoy, int speed, int move_range, int stage, int look, bool show_mob = 1, bool activated = 1) {
    monster->obj = createObject(c);
    locateObject(monster->obj, back[stage], monster->x = x, monster->y = y);
    monster->xtox = xtox;
    monster->ytoy = ytoy;
    monster->speed = speed;
    monster->look = look;
    monster->move_range = move_range;
    monster->stage = stage;
    monster->move_max_cnt = monster->move_range / monster->speed;
    monster->activated = activated;
    monster->move_now_cnt = 0;
    if (show_mob) showObject(monster->obj);
    else hideObject(monster->obj);
}

//몬스터 판정
//조금이라도 닿으면 맞는 판정임.
void check_hit(mob* monster, int stage) {
    if (stage == monster->stage) {
        if ((monster->y < player1.y + player1.ytoy && monster->y + monster->ytoy >player1.y) && (monster->x < player1.x + player1.xtox && monster->x + monster->xtox > player1.x)) {
            game_over();
        }

    }
}

void move_monster(mob* monster, int stage) {
    if (monster->activated == 0)
    {
        return;
    }

    if (stage == monster->stage) {
        //오른쪽으로 바라봄
        monster->move_now_cnt += 1;
        if (monster->look == 1) {
            locateObject(monster->obj, back[stage], monster->x += monster->speed, monster->y);
            if (monster->walk_state == 0) {
                monster->walk_state = 1;

            }

            else {
                monster->walk_state = 0;

            }
        }

        else if (monster->look == -1) {

            locateObject(monster->obj, back[stage], monster->x -= monster->speed, monster->y);
            if (monster->walk_state == 0) {
                monster->walk_state = 1;

            }

            else {
                monster->walk_state = 0;

            }
        }




        if (monster->move_now_cnt >= monster->move_max_cnt) {
            monster->move_now_cnt = 0;
            if (monster->look == 1) monster->look = -1;
            else monster->look = 1;
        }

    }



}

void next_script() {
    

    hideObject(script[script_num]);
    script_num++;
    showObject(script[script_num]);

    
}

void change_monster_img(mob* monster, int stage) {
    if (stage == monster->stage) {
        if (1)
        {
            //오른쪽
            if (monster->look == 1) {
                if (monster->walk_state == 0) {
                    setObjectImage(monster->obj, "rat_right_1.png");
                }

                else {
                    setObjectImage(monster->obj, "rat_right_2.png");
                }
            }

            else if (monster->look == -1) {
                if (monster->walk_state == 0) {
                    setObjectImage(monster->obj, "rat_left_1.png");
                }

                else {
                    setObjectImage(monster->obj, "rat_left_2.png");
                }
            }
        }


    }
}

void player_walk() {
    //정지
    if (player_vec == 0) {
        setObjectImage(player1.obj, "player_walk_stop.png");
    }


    //오른쪽
    else if (player_vec == 1) {
        //printf("%d", walk_state);
        if (walk_state == 1) {
            setObjectImage(player1.obj, "player_walk_2.png");
            walk_state = 0;

        }
        else {
            setObjectImage(player1.obj, "player_walk_1.png");
            walk_state = 1;
        }

    }

    //왼쪽
    else if (player_vec == -1) {
        if (walk_state == 1) {
            setObjectImage(player1.obj, "player_walk_1_L.png");
            walk_state = 0;

        }

        else {
            setObjectImage(player1.obj, "player_walk_2_L.png");
            walk_state = 1;
        }
    }
}

//파이어볼
void moveBullet()
{
    for (int i = 0; i < ableFireball; i++)
    {
        if (bullet[i].activated == 1)
        {
            locateObject(bullet[i].obj, back[scene_num], bullet[i].x += bullet[i].vector * BULLETSPEED, bullet[i].y);

            if (bullet[i].vector == 1 && bullet[i].x > bullet[i].endX)
            {
                bullet[i].activated = 0;
                hideObject(bullet[i].obj);

            }
            if (bullet[i].vector == -1 && bullet[i].x < bullet[i].endX)
            {

                bullet[i].activated = 0;
                hideObject(bullet[i].obj);

            }

            for (int j = 0; j < stage_by_mob[scene_num]; j++)
            {

                if (bullet[i].x + SIZEOFFIREBALL > monster[j].x && monster[j].activated == 1 && bullet[i].x < monster[j].x + monster[j].xtox && bullet[i].activated == 1 && (bullet[i].y < monster[j].y + monster[j].ytoy && bullet[i].y + SIZEOFFIREBALL>monster[j].y))
                {
                    bullet[i].activated = 0;
                    hideObject(bullet[i].obj);
                    monster[j].activated = 0;
                    hideObject(monster[j].obj);
                }
            }




        }


    }

}

void shot(player* agent)
{
    int find = -1;
    int temp = -999;
    for (int i = 0; i < ableFireball; i++)
    {
        if (bullet[i].activated == 0)
        {
            find = i;
            break;
        }
    }
    if (find == -1)
    {
        return;
    }
    //함수 시작부터 여까지는 어떤 투사체가 사용가능한지 찾는 코드
    locateObject(bullet[find].obj, back[scene_num], bullet[find].x = (agent->x + agent->xtox / 2) - SIZEOFFIREBALL / 2, bullet[find].y = (agent->y + agent->ytoy / 2) - SIZEOFFIREBALL / 2);
    showObject(bullet[find].obj);


    if (agent->look)  bullet[find].vector = 1;
    else bullet[find].vector = -1;


    bullet[find].activated = 1;
    for (int i = 0; i < stage_by_wall[scene_num]; i++)
    {

        if ((bullet[find].y < wall1[i].y + wall1[i].ytoy && bullet[find].y + SIZEOFFIREBALL>wall1[i].y) == 0)
        {

            continue;
        }

        if (agent->look && bullet[find].x < wall1[i].x)
        {
            if (temp == -999) temp = wall1[i].x - SIZEOFFIREBALL;
            if (temp > wall1[i].x - SIZEOFFIREBALL)
            {
                temp = wall1[i].x - SIZEOFFIREBALL;
            }

        }
        if (agent->look == 0 && bullet[find].x + SIZEOFFIREBALL > wall1[i].x + wall1[i].xtox)
        {
            if (temp == -999) temp = wall1[i].x + wall1[i].xtox;
            if (temp < wall1[i].x + wall1[i].xtox)
            {
                temp = wall1[i].x + wall1[i].xtox;
            }
            printf("\n %d", i);
        }

    }
    bullet[find].endX = temp;

    if (bullet[find].endX == -999)
    {
        if (agent->look) bullet[find].endX = 1280 - SIZEOFFIREBALL;
        else bullet[find].endX = 0;
    }


}

//초기설정(타이머, 배경, 플레이어 속성)
void game_init() {
    setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
    setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);
    setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);

    //callback함수 선언
    setTimerCallback(timerCallback);
    setKeyboardCallback(keyboardCallback);
    setMouseCallback(mouseCallback);

    //배경 생성
    make_back();

    //스크립트 생성
    make_script();

    //포탈 생성
    make_portal();

    //벽 생성
    makeWall();

    //몬스터 생성
    make_monster();

    //버튼들 생성
    make_button();

    //기본 타이머
    frame = createTimer(0.1f);
    startTimer(frame);
    walk_timer = createTimer(0.01f);
    startTimer(walk_timer);
    walk_monster = createTimer(0.01f);
    startTimer(walk_monster);
    lion_timer = createTimer(30.f);

    //플레이어
    player1.obj = createObject("player.png");
    locateObject(player1.obj, back[0], player1.x = spawn_x[0], player1.y = spawn_y[0]);
    showObject(player1.obj);

    //투사체 생성
    for (int i = 0; i < 5; i++) {
        bullet[i].obj = createObject("shot.png");
        locateObject(bullet[i].obj, back[0], 640, 360);
        hideObject(bullet[i].obj);
    }

    //플레이어 너비, 높이
    player1.xtox = 30;
    player1.ytoy = 30;
}

//클리어 조건(xtox = 50, ytoy = 100)
void check_clear(int stage) {

    if ((player1.x + player1.xtox >= portal_x[stage] && player1.x <= portal_x[stage] + portal_xtox) && (player1.y >= portal_y[stage] && player1.y + player1.ytoy <= portal_y[stage] + portal_ytoy)) {
        next_stage2();
    }
}


//게임오버
void game_over() {
    input_key = false;
    //printf("scene: %d \n", scene_num);
    for (int i = 0; i < stage_by_mob[scene_num]; i++)
    {
        hideObject(monster[i].obj);
        printf("\ndel");
    }
    hideTimer();
    enterScene(game_over_back);
    locateObject(restart_button, game_over_back, 930, 530);
    scaleObject(restart_button, 1.5f);
    showObject(restart_button);
}


int main()
{
    //초기 설정
    game_init();
    startGame(main_scene);

}



//이동함수 x
void move_x(player* agent, int stage)
{
    int sit = -1; //상황 저장용 -1:움직일 수 있음 / 그 외:벽sit번째 때문에 움직일 수 없음
    int vector = 0; //방향 나타냄 0정지, 1오른쪽, -1왼쪽)

    if (agent->speed_x == 0)
    {
        vector = 0;
        player_vec = 0;
    }
    else
    {
        vector = agent->speed_x / abs(agent->speed_x);
        player_vec = vector;
    }


    sit = checkPlayer_AllWallToMoveX(agent, wall1, vector);
    if (sit == -1)
    {
        locateObject(agent->obj, back[stage], agent->x += agent->speed_x, agent->y);
        if (stuck)
        {
            stuck = false;

        }

        if (vector == -1) agent->look = 0;
        else if (vector == 1) agent->look = 1;

    }

    else
    {
        if (vector == 1)
        {

            locateObject(agent->obj, back[stage], agent->x += (wall1[sit].x - (agent->x + agent->xtox)), agent->y);
            if (agent->speed_y < 0)
            {
                stuck = true;
                agent->look = 0;
            }
            agent->jumpState = 0;

        }
        if (vector == -1)
        {
            locateObject(agent->obj, back[stage], agent->x -= (agent->x - (wall1[sit].x + wall1[sit].xtox)), agent->y);
            if (agent->speed_y < 0)
            {
                stuck = true;
                agent->look = 1;
            }
            agent->jumpState = 0;

        }
    }
    if (stuck)
    {
        agent->speed_y = -STUCK_SPEED;
    }

}

//벽 체크 x
int checkPlayer_AllWallToMoveX(player* obj1, wall obj2[], int obj1Vector)
{
    if (obj1Vector == 0)
    {
        return -1;
    }
    //printf("\n%d", obj1Vector);
    int targetWall = -1;   //비어있음을 나타내는 -1
    for (int i = 0; i < stage_by_wall[scene_num]; i++)
    {
        bool state = (obj1->y<obj2[i].y + obj2[i].ytoy && obj1->y + obj1->ytoy>obj2[i].y);
        //printf("\nstate : %d", state);
        if (state == false)
        {
            continue;
        }
        if ((targetWall == -1))
        {
            if (obj1Vector == 1 && (obj1->x + obj1->xtox + obj1->speed_x) > obj2[i].x && (obj1->x + obj1->xtox) <= obj2[i].x)
            {
                targetWall = i;
            }
            else if (obj1Vector == -1 && (obj1->x + obj1->speed_x) < (obj2[i].x + obj2[i].xtox) && (obj1->x) >= (obj2[i].x + obj2[i].xtox))
            {
                targetWall = i;
            }
            continue;
        }

        if (obj1Vector == 1 && obj1->x + obj1->xtox + obj1->speed_x > obj2[i].x && (obj1->x + obj1->xtox) <= obj2[i].x)
        {
            if (obj2[targetWall].x > obj2[i].x) targetWall = i;
        }
        else if (obj1Vector == -1 && obj1->x + obj1->speed_x < obj2[i].x + obj2[i].xtox && (obj1->x) >= (obj2[i].x + obj2[i].xtox))
        {
            if (obj2[targetWall].x + obj2[targetWall].xtox < obj2[i].x + obj2[i].xtox) targetWall = i;
        }

    }
    //printf("\ntargetwall : %d", targetWall);
    return targetWall;

}

//이동함수 y
void move_y(player* agent, int stage)
{
    int sit = -1; //상황 저장용 -1:움직일 수 있음 / 그 외:벽sit번째 때문에 움직일 수 없음
    int vector = 0; //방향 나타냄 0정지, 1위쪽, -1아래쪽)

    if (agent->speed_y == 0)
    {
        vector = 0;
    }
    else
    {
        vector = agent->speed_y / abs(agent->speed_y);
    }


    sit = checkPlayer_AllWallToMoveY(agent, wall1, vector);

    if (sit == -1)
    {
        locateObject(agent->obj, back[stage], agent->x, agent->y += agent->speed_y);
    }
    else
    {
        if (vector == 1)
        {
            locateObject(agent->obj, back[stage], agent->x, agent->y += (wall1[sit].y - (agent->y + agent->ytoy)));
            agent->speed_y = 0;

        }
        if (vector == -1)
        {
            locateObject(agent->obj, back[stage], agent->x, agent->y -= (agent->y - (wall1[sit].y + wall1[sit].ytoy)));
            agent->speed_y = 0;
            agent->jumpState = 0;
            if (stuck == 1)
            {
                agent->look = !agent->look;
                stuck = 0;
            }

        }
    }


}

//벽체크 y
int checkPlayer_AllWallToMoveY(player* obj1, wall obj2[], int obj1Vector)
{
    if (obj1Vector == 0)
    {
        return -1;
    }

    int targetWall = -1;   //비어있음을 나타내는 -1
    for (int i = 0; i < stage_by_wall[scene_num]; i++)
    {
        bool state = ((obj1->x<obj2[i].x + obj2[i].xtox && obj1->x + obj1->xtox > obj2[i].x));
        //printf("\n%d state : %d",i, state);
        if (state == false)
        {
            continue;
        }
        if ((targetWall == -1))
        {
            if (obj1Vector == 1 && (obj1->y + obj1->ytoy + obj1->speed_y) > obj2[i].y && (obj1->y + obj1->ytoy) <= obj2[i].y)
            {
                targetWall = i;
            }
            else if (obj1Vector == -1 && (obj1->y + obj1->speed_y) < (obj2[i].y + obj2[i].ytoy) && (obj1->y) >= (obj2[i].y + obj2[i].ytoy))
            {
                targetWall = i;
            }
            continue;
        }

        if (obj1Vector == 1 && obj1->y + obj1->ytoy + obj1->speed_y < obj2[i].y && (obj1->y + obj1->ytoy) <= obj2[i].y)
        {
            if (obj2[targetWall].y > obj2[i].y) targetWall = i;
        }
        else if (obj1Vector == -1 && obj1->y + obj1->speed_y < obj2[i].y + obj2[i].ytoy && (obj1->y) >= (obj2[i].y + obj2[i].ytoy))
        {
            if (obj2[targetWall].y + obj2[targetWall].ytoy < obj2[i].y + obj2[i].ytoy) targetWall = i;
        }

    }
    //printf("\ntargetwall : %d", targetWall);
    return targetWall;

}

//wall 구조체, img 파일명, x, y, 너비, 높이, 배경 stage, bool showimg
void createWall(wall* w, const char* c, int x, int y, int xtox, int ytoy, int stage, bool show = 1)
{

    w->obj = createObject(c);
    locateObject(w->obj, back[stage], w->x = x, w->y = y);
    w->xtox = xtox;
    w->ytoy = ytoy;
    if (show) showObject(w->obj);
    else hideObject(w->obj);

}

//벽 생성
void makeWall()
{
    //stage = 0 튜토리얼
    if (scene_num == 0) {
        createWall(wall1 + 0, "wall.png", 0, 0, 1280, 20, 0);
        createWall(wall1 + 1, "tutorial_tree.png", 590, 20, 100, 200, 0);
        createWall(wall1 + 2, "tutorial_tree.png", 590, 420, 100, 200, 0);
        createWall(wall1 + 3, "tutorial_tree.png", 590, 620, 100, 200, 0);
    }

    //stage = 1 허수아비
    else if (scene_num == 1) {
        createWall(wall1 + 0, "wall.png", 0, 0, 1280, 20, 1);
        createWall(wall1 + 1, "long_wood.png", 330, 20, 100, 200, 1);
        createWall(wall1 + 2, "long_wood.png", 330, 220, 100, 200, 1);
        createWall(wall1 + 3, "long_wood.png", 330, 420, 100, 200, 1);
        createWall(wall1 + 4, "slim_wood_y.png", 590, 20, 25, 200, 1);
        createWall(wall1 + 5, "slim_wood_y.png", 590, 420, 25, 200, 1);
        createWall(wall1 + 6, "slim_wood_y.png", 590, 620, 25, 200, 1);
        createWall(wall1 + 7, "slim_wood_y.png", 1000, 20, 25, 200, 1);
        createWall(wall1 + 8, "slim_wood_y.png", 1000, 220, 25, 200, 1);
        createWall(wall1 + 9, "slim_wood_y.png", 1000, 420, 25, 200, 1);
        createWall(wall1 + 10, "short_wood.png", 680, 270, 100, 50, 1);
        createWall(wall1 + 11, "short_wood.png", 850, 420, 100, 50, 1);
    }

    //stage = 2-1 오두막으로
    else if (scene_num == 2) {
        createWall(wall1 + 0, "short_wall.png", 0, 0, 100, 20, 2);
        createWall(wall1 + 1, "long_wood.png", 180, 20, 100, 200, 2);
        createWall(wall1 + 2, "long_wood.png", 180, 220, 100, 200, 2);
        createWall(wall1 + 3, "long_wood.png", 180, 300, 100, 200, 2);
        createWall(wall1 + 4, "slim_wood_x.png", 400, 380, 100, 25, 2);
        createWall(wall1 + 5, "slim_wood_x.png", 420, 450, 100, 25, 2);
        createWall(wall1 + 6, "slim_wood_x.png", 420, 380, 100, 25, 2);
        createWall(wall1 + 7, "slim_wood_x.png", 550, 320, 100, 25, 2);
        createWall(wall1 + 8, "slim_wood_x.png", 720, 420, 100, 25, 2);
        createWall(wall1 + 9, "slim_wood_y.png", 1000, 20, 25, 200, 2);
        createWall(wall1 + 10, "slim_wood_y.png", 1000, 220, 25, 200, 2);
        createWall(wall1 + 11, "slim_wood_y.png", 1000, 420, 25, 200, 2);
        createWall(wall1 + 12, "short_wall.png", 1180, 0, 100, 20, 2);
        createWall(wall1 + 13, "short_wall.png", 1080, 0, 100, 20, 2);

    }

    //stage = 2-2 기름통을 찾아서
    else if (scene_num == 3) {
        createWall(wall1 + 0, "short_wall.png", 0, 0, 100, 20, 3);
        createWall(wall1 + 1, "long_wood.png", 150, 20, 100, 200, 3);
        createWall(wall1 + 2, "long_wood.png", 150, 220, 100, 200, 3);
        createWall(wall1 + 3, "long_wood.png", 370, 20, 100, 200, 3);
        createWall(wall1 + 4, "long_wood.png", 370, 120, 100, 200, 3);
        createWall(wall1 + 5, "slim_wood_y.png", 590, 20, 25, 200, 3);
        createWall(wall1 + 6, "slim_wood_y.png", 590, 190, 25, 200, 3);
        createWall(wall1 + 7, "long_wood.png", 800, 20, 100, 200, 3);
        createWall(wall1 + 8, "slim_wood_y.png", 1000, 220, 25, 200, 3);
        createWall(wall1 + 9, "slim_wood_y.png", 1000, 420, 25, 200, 3);
        createWall(wall1 + 10, "short_wall.png", 1180, 0, 100, 20, 3);
        createWall(wall1 + 11, "short_wall.png", 1080, 0, 100, 20, 3);
    }

    //stage = 3-1 몹 피하기 튜토리얼
    else if (scene_num == 4) {
        createWall(wall1 + 0, "wall.png", 0, 0, 1280, 20, 4);
        createWall(wall1 + 1, "short_wood.png", 400, 100, 100, 50, 4);
        createWall(wall1 + 2, "short_wood.png", 500, 100, 100, 50, 4);
        createWall(wall1 + 3, "short_wood.png", 600, 100, 100, 50, 4);
    }

    //stage = 3-2 겁쟁이사자 맵
    else if (scene_num == 5) {
        createWall(wall1 + 0, "wall.png", 0, 0, 1280, 20, 5);
        createWall(wall1 + 1, "long_wood.png", 300, 20, 100, 200, 5);
        createWall(wall1 + 2, "long_wood.png", 300, 220, 100, 200, 5);
        createWall(wall1 + 3, "long_wood.png", 800, 20, 100, 200, 5);
        createWall(wall1 + 4, "long_wood.png", 800, 220, 100, 200, 5);
        createWall(wall1 + 5, "short_wood.png", 500, 120, 100, 50, 5);
        createWall(wall1 + 6, "short_wood.png", 700, 220, 100, 50, 5);
        createWall(wall1 + 7, "short_wood.png", 900, 320, 100, 50, 5);
        createWall(wall1 + 8, "short_wood.png", 1100, 370, 100, 50, 5);
    }

    //stage = 3-3 겁쟁이사자 맵
    else if (scene_num == 6) {
        createWall(wall1 + 0, "short_wall.png", 0, 0, 100, 20, 6);
        createWall(wall1 + 1, "wall.png", 0, 720, 1280, 20, 6);
        createWall(wall1 + 2, "slim_wood_x.png", 150, 100, 100, 25, 6);
        createWall(wall1 + 3, "slim_wood_x.png", 250, 100, 100, 25, 6);
        createWall(wall1 + 4, "slim_wood_y.png", 150, 200, 25, 200, 6);
        createWall(wall1 + 5, "slim_wood_y.png", 150, 400, 25, 200, 6);
        createWall(wall1 + 6, "slim_wood_y.png", 325, 200, 25, 200, 6);
        createWall(wall1 + 7, "slim_wood_y.png", 325, 400, 25, 200, 6);
        createWall(wall1 + 8, "slim_wood_x.png", 400, 500, 100, 25, 6);
        createWall(wall1 + 9, "slim_wood_y.png", 600, 400, 25, 200, 6);
        createWall(wall1 + 10, "slim_wood_y.png", 600, 200, 25, 200, 6);
        createWall(wall1 + 11, "slim_wood_x.png", 850, 250, 100, 25, 6);
        createWall(wall1 + 12, "slim_wood_x.png", 750, 250, 100, 25, 6);
        createWall(wall1 + 13, "slim_wood_x.png", 950, 250, 100, 25, 6);
        createWall(wall1 + 14, "slim_wood_x.png", 1000, 120, 100, 25, 6);
        createWall(wall1 + 15, "slim_wood_x.png", 1100, 120, 100, 25, 6);
        createWall(wall1 + 16, "slim_wood_x.png", 1200, 120, 100, 25, 6);
        createWall(wall1 + 17, "wall.png", 1000, 0, 180, 20, 6);




    }

    //stage = 3-4 겁쟁이사자 맵
    else if (scene_num == 7) {
        createWall(wall1 + 0, "short_wall.png", 0, 400, 100, 20, 7);
        createWall(wall1 + 1, "wall.png", 0, 720, 1280, 20, 7);
        createWall(wall1 + 2, "long_slim_wood_x.png", 0, 300, 200, 25, 7);
        createWall(wall1 + 3, "long_slim_wood_x.png", 200, 300, 200, 25, 7);
        createWall(wall1 + 4, "long_slim_wood_x.png", 400, 300, 200, 25, 7);
        createWall(wall1 + 5, "long_slim_wood_x.png", 600, 300, 200, 25, 7);
        createWall(wall1 + 6, "long_slim_wood_x.png", 800, 300, 200, 25, 7);
        createWall(wall1 + 7, "slim_wood_x.png", 300, 400, 100, 25, 7);
        createWall(wall1 + 8, "slim_wood_x.png", 400, 400, 100, 25, 7);
        createWall(wall1 + 9, "wood_block.png", 375, 500, 50, 50, 7);

        createWall(wall1 + 10, "slim_wood_x.png", 600, 500, 100, 25, 7);
        createWall(wall1 + 11, "slim_wood_y.png", 975, 300, 25, 200, 7);
        createWall(wall1 + 12, "wood_block.png", 800, 600, 50, 50, 7);
        createWall(wall1 + 13, "slim_wood_x.png", 1100, 200, 100, 25, 7);
        createWall(wall1 + 14, "slim_wood_x.png", 900, 100, 100, 25, 7);
        createWall(wall1 + 15, "wood_block.png", 800, 100, 50, 50, 7);
        createWall(wall1 + 16, "short_slim_wood_y.png", 600, 100, 25, 100, 7);
        createWall(wall1 + 17, "small_wood_block.png", 500, 20, 25, 25, 7);
        createWall(wall1 + 18, "small_wood_block.png", 400, 100, 25, 25, 7);
        createWall(wall1 + 19, "long_slim_wood_x.png", 100, 50, 200, 25, 7);
    }

    //stage = 4 화염구 튜토리얼
    else if (scene_num == 8) {
        createWall(wall1 + 0, "wall.png", -700, 0, 1280, 20, 8);
        createWall(wall1 + 1, "Fstone.png", 600, 100, 320, 40, 8);
        createWall(wall1 + 2, "Fstone.png", 445, 220, 320, 40, 8);//
        createWall(wall1 + 3, "Fstone.png", 550, 340, 320, 40, 8);//
        createWall(wall1 + 4, "Fstone.png", 870, 340, 320, 40, 8);//
        createWall(wall1 + 5, "Fstone.png", 100, 460, 320, 40, 8);
        createWall(wall1 + 6, "Fstone.png", 520, 460, 320, 40, 8);
        createWall(wall1 + 7, "Fstone_short.png", -10, 280, 160, 40, 8);
    }
    //stage = 5-1 순찰봇 잡기
    else if (scene_num == 9) {
        createWall(wall1 + 0, "wall.png", -1200, 0, 1280, 20, 9);
        createWall(wall1 + 1, "Fstone_short2.png", 100, 80, 80, 40, 9);
        createWall(wall1 + 2, "Fstone_short2.png", 130, 180, 80, 40, 9);
        createWall(wall1 + 3, "Fstone_short2.png", 100, 280, 80, 40, 9);
        createWall(wall1 + 4, "Fstone_short2.png", 250, 420, 80, 40, 9);
        createWall(wall1 + 5, "Fstone_short2.png", 100, 560, 80, 40, 9);
        createWall(wall1 + 6, "FstoneL.png", 400, 350, 40, 320, 9);
        createWall(wall1 + 7, "FstoneL.png", 480, 500, 40, 320, 9);
        createWall(wall1 + 8, "Fstone_short2.png", 300, 630, 80, 40, 9);
        createWall(wall1 + 9, "Fstone.png", 700, 220, 320, 40, 9);
        createWall(wall1 + 10, "Fstone_shortS.png", 600, 260, 80, 80, 9);
        createWall(wall1 + 11, "Fstone_short2.png", 300, 820, 180, 40, 9);
        createWall(wall1 + 12, "FstoneL.png", 480, 420, 40, 320, 9);
        createWall(wall1 + 13, "Fstone.png", 900, 0, 320, 40, 9);
    }
    //stage = 5-2 스테이지 추가
    else if (scene_num == 10) {
        createWall(wall1 + 0, "Fstone_short2.png", 580, 310, 80, 40, 10);
        createWall(wall1 + 1, "Fstone.png", 900, 150, 320, 40, 10);
        createWall(wall1 + 2, "FstoneL.png", 685, 300, 40, 320, 10);
        createWall(wall1 + 3, "Fstone_short.png", 500, 530, 160, 40, 10);
        createWall(wall1 + 4, "Fstone_short.png", 130, 200, 160, 40, 10);
        createWall(wall1 + 5, "Fstone.png", 20, 0, 320, 40, 10);
        createWall(wall1 + 6, "FstoneL.png", 120, 140, 40, 320, 10);
        createWall(wall1 + 7, "Fstone.png", 340, 0, 320, 40, 10);
        createWall(wall1 + 8, "FstoneL.png", 120, 270, 40, 320, 10);
        createWall(wall1 + 9, "Fstone_short.png", 320, 530, 160, 40, 10);
        createWall(wall1 + 10, "Fstone_short.png", 140, 530, 160, 40, 10);
        createWall(wall1 + 11, "Fstone_short.png", 720, 600, 160, 40, 10);
        createWall(wall1 + 12, "Fstone_short.png", 1120,-20, 160, 40, 10);
        
    }



}


void make_monster() {
    //mob* monster, const char* c, int x, int y, int xtox, int ytoy, int speed, int move_range, int stage, int look, bool show_mob = 1, bool activated
    if (scene_num == 4) {
        spawn_monster(monster + 0, "rat_right_1.png", 500, 20, 80, 70, 1, 100, scene_num, 1, 1, 1);

    }

    else if (scene_num == 5) {
        spawn_monster(monster + 0, "rat_right_1.png", 450, 20, 80, 70, 3, 220, scene_num, 1, 1, 1);
        spawn_monster(monster + 1, "rat_right_1.png", 900, 20, 80, 70, 3, 300, scene_num, 1, 1, 1);
    }

    else if (scene_num == 6) {
        spawn_monster(monster + 0, "rat_right_1.png", 150, 125, 80, 70, 1, 120, scene_num, 1, 1, 1);
        spawn_monster(monster + 1, "rat_right_1.png", 750, 275, 80, 70, 2, 220, scene_num, 1, 1, 1);
    }

    else if (scene_num == 7) {
        spawn_monster(monster + 0, "rat_right_1.png", 0, 325, 80, 70, 4, 220, scene_num, 1, 1, 1);
        spawn_monster(monster + 1, "rat_right_1.png", 300, 325, 80, 70, 4, 220, scene_num, 1, 1, 1);
        spawn_monster(monster + 2, "rat_right_1.png", 600, 325, 80, 70, 5, 295, scene_num, 1, 1, 1);
        spawn_monster(monster + 3, "rat_right_1.png", 300, 425, 80, 70, 2, 120, scene_num, 1, 1, 1);


    }

    else if (scene_num == 8) {
        spawn_monster(monster + 0, "rat_right_1.png", 150, 20, 80, 70, 3, 220, scene_num, 1, 1, 1);
        spawn_monster(monster + 1, "rat_right_1.png", 600, 140, 80, 70, 2, 260, scene_num, 1, 1, 1);
        spawn_monster(monster + 2, "rat_right_1.png", 450, 260, 80, 70, 1, 250, scene_num, 1, 1, 1);
        spawn_monster(monster + 3, "rat_right_1.png", 600, 380, 80, 70, 4, 400, scene_num, 1, 1, 1);
        spawn_monster(monster + 4, "rat_right_1.png", 150, 500, 80, 70, 4, 100, scene_num, 1, 1, 1);
    }
    else if (scene_num == 9) {
        spawn_monster(monster + 0, "rat_right_1.png", 600, 340, 80, 70, 1, 15, scene_num, 1, 1, 1);
        spawn_monster(monster + 1, "rat_right_1.png", 700, 260, 80, 70, 3, 220, scene_num, 1, 1, 1);
        spawn_monster(monster + 2, "rat_right_1.png", 700, 260, 80, 70, 4, 220, scene_num, 1, 1, 1);
        spawn_monster(monster + 3, "rat_right_1.png", 920, 260, 80, 70, 3, 220, scene_num, -1, 1, 1);
    }
    else if (scene_num == 10) {
        spawn_monster(monster + 0, "rat_right_1.png", 880, 190, 80, 70, 10, 280, scene_num, 1, 1, 1);
        spawn_monster(monster + 1, "rat_right_1.png", 920, 190, 80, 70, 20, 140, scene_num, 1, 1, 1);
        spawn_monster(monster + 2, "rat_right_1.png", 1060, 190, 80, 70, 20, 140, scene_num, 1, 1, 1);
    }
}