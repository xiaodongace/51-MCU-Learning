#include "BuzzerSequenceLab.h"
#include "Key.h"
#include "Buzzer.h"
#include "LED.h"
#include "Timers.h"

/* 旋律中共有三个音符。 */
#define NOTE_COUNT 3

/* Buzzer_Beep() 接收的是预设音调编号，而不是频率值。 */
#define NOTE_C5 1
#define NOTE_D5 2
#define NOTE_E5 3

/*
 * 旋律状态机：
 * SEQUENCE_IDLE：空闲，没有播放；
 * SEQUENCE_NOTE：正在播放当前音符；
 * SEQUENCE_GAP：当前音符结束后的50 ms间隔。
 * SEQUENCE_PAUSED：暂停
 */
#define SEQUENCE_IDLE  0
#define SEQUENCE_NOTE  1
#define SEQUENCE_GAP   2
#define SEQUENCE_PAUSED 3



/*
 * 旋律数组放在 code 区，减少对8051内部 DATA 空间的占用。
 * 数组中的值是 Buzzer_Beep() 使用的音调编号。
 */
static u8 code notes[NOTE_COUNT] = {
    NOTE_C5,
    NOTE_D5,
    NOTE_E5
};

/* 播放状态和当前音符下标。 */
static u8 is_playing;
static u8 note_index;

/* 暂停前处于什么状态 */
static u8 state_before_pause;
/* 该状态已经经过多少毫秒 */
static u16 paused_elapsed_ms;

/* 状态机当前状态，以及当前状态开始的系统时间。 */
static u8 sequence_state;
static u16 sequence_start_ms;

/*
 * 开始播放旋律。
 *
 * 这里只建立初始状态并立即播放第一个音符，不等待300 ms；
 * 音符持续时间由 BuzzerSequenceLab_PlaySequence() 检查。
 */
static void BuzzerSequenceLab_Start(void) {
    u16 now = Timers_GetSystemMs();

    is_playing = 1;
    note_index = 0;
    sequence_state = SEQUENCE_NOTE;
    sequence_start_ms = now;

    LED_SetAll(1);
    Buzzer_Beep(notes[note_index]);
}

/*
 * 停止旋律并恢复到空闲状态。
 *
 * KEY2停止和最后一个音符播放完成都调用这个函数，
 * 统一关闭蜂鸣器、关闭LED并清零播放位置。
 */
static void BuzzerSequenceLab_Stop(void) {
    is_playing = 0;
    note_index = 0;
    sequence_state = SEQUENCE_IDLE;
    sequence_start_ms = 0;

    Buzzer_Stop();
    LED_SetAll(0);
}

/*
 * 非阻塞地推进旋律状态机。
 *
 * 每次调用最多完成一次状态转换，然后立即返回：
 * NOTE持续300 ms后进入GAP；
 * GAP持续50 ms后播放下一个音符或结束旋律。
 */
static void BuzzerSequenceLab_PlaySequence(void) {
    u16 now = Timers_GetSystemMs();

    if (sequence_state == SEQUENCE_NOTE) {
        /* 当前音符播放满300 ms，关闭声音并进入间隔状态。 */
        if (now - sequence_start_ms >= 300) {
            Buzzer_Stop();
            sequence_start_ms = now;
            sequence_state = SEQUENCE_GAP;
        }
    }
    else if (sequence_state == SEQUENCE_GAP) {
        /* 间隔满50 ms后，移动到下一个音符。 */
        if (now - sequence_start_ms >= 50) {
            note_index++;

            if (note_index >= NOTE_COUNT) {
                BuzzerSequenceLab_Stop();
            }
            else {
                Buzzer_Beep(notes[note_index]);
                sequence_start_ms = now;
                sequence_state = SEQUENCE_NOTE;
            }
        }
    }
}

/*
 * 处理已经经过消抖确认的按下事件。
 *
 * KEY1：空闲时启动旋律，播放期间忽略；
 * KEY2：NOTE和GAP状态都可以停止旋律。
 */
static void BuzzerSequenceLab_HandleKeys(void) {
    /* 公共Key模块负责扫描、消抖并锁存一次性按下事件。 */
    Key_Scan();

    /* 读取事件时自动清零，KEY1按住期间不会重复启动。 */
    if (Key_GetPressEvent(0) != 0) {
        if (sequence_state == SEQUENCE_IDLE) {
            BuzzerSequenceLab_Start();
        }
    }

    /* 旧版流程使用KEY2结束当前旋律。 */
    if (Key_GetPressEvent(1) != 0) {
        if (sequence_state != SEQUENCE_IDLE) {
            BuzzerSequenceLab_Stop();
        }
    }
}

/* 初始化状态、输入输出外设和系统定时器。 */
void BuzzerSequenceLab_Init(void) {
    /* 先初始化状态，避免定时器启动后访问未初始化的数据。 */
    is_playing = 0;
    note_index = 0;
    sequence_state = SEQUENCE_IDLE;
    sequence_start_ms = 0;
    state_before_pause = SEQUENCE_IDLE;
    paused_elapsed_ms = 0;

    LED_Init();
    Key_Init();
    Buzzer_Init();

    /* 建立确定的上电输出状态。 */
    LED_SetAll(0);
    Buzzer_Stop();

    /* 最后启动Timer0，之后 system_ms 才会开始递增。 */
    Timers_Init();
}

/*
 * KEY1：C5 → D5 → E5，每个音符300 ms，间隔50 ms；
 * 播放期间LED全亮，结束或KEY2停止后蜂鸣器关闭、LED全灭。
 */
void BuzzerSequenceLab_Task(void) {
    /* 每次主循环先处理按键，再推进一次旋律状态机。 */
    BuzzerSequenceLab_HandleKeys();

    if (is_playing != 0) {
        BuzzerSequenceLab_PlaySequence();
    }
}





/*
 * 从第一个音符开始一次新的旋律播放。
 * 本函数只建立初始状态并启动第一个音符，不在这里等待音符结束。
 */
void New_SequenceStart(void) {
    /* 用当前系统时间作为第一个NOTE状态的计时起点。 */
    u16 now = Timers_GetSystemMs();

    /* 标记旋律已经启动，并确保每次都从notes[0]开始。 */
    is_playing = 1;
    note_index = 0;
    sequence_state = SEQUENCE_NOTE;
    sequence_start_ms = now;

    /* 播放期间点亮LED，并立即发出第一个音符。 */
    LED_SetAll(1);
    Buzzer_Beep(notes[note_index]);
}

/*
 * 停止当前旋律并恢复基本的空闲状态。
 * 自然播放结束和K3主动停止都通过本函数统一关闭输出、清除播放进度。
 */
void New_SequenceStop(void) {
    /* 清除基本播放状态，使下一次启动重新从第一个音符开始。 */
    is_playing = 0;
    note_index = 0;
    sequence_state = SEQUENCE_IDLE;
    sequence_start_ms = 0;

    /* 清除暂停数据 */
    paused_elapsed_ms = 0;
    state_before_pause = SEQUENCE_IDLE;

    /* 明确关闭两个输出，保证IDLE状态没有残留声音或灯光。 */
    LED_SetAll(0);
    Buzzer_Stop();
}

/*
 * 根据当前状态和经过时间，非阻塞地推进旋律播放。
 * NOTE满300 ms后进入GAP；GAP满50 ms后播放下一音符或结束旋律。
 */
void New_PlaySequence(void) {
    /* 本轮只读取一次时间，保证所有判断使用相同的时间快照。 */
    u16 now = Timers_GetSystemMs();

    /* NOTE状态只检查当前音符是否已经播放满300 ms。 */
    if (sequence_state == SEQUENCE_NOTE) {
        if (now - sequence_start_ms >= 300) {
            /* 音符到期：停止声音，并从此刻开始计算静音间隔。 */
            Buzzer_Stop();
            sequence_start_ms = now;
            sequence_state = SEQUENCE_GAP;
        }
    }
    /* 与NOTE互斥，保证每次调用最多完成一次状态转换。 */
    else if (sequence_state == SEQUENCE_GAP && now - sequence_start_ms >= 50) {
        /* 间隔到期后，把下标移动到候选的下一个音符。 */
        note_index++;

        if (note_index >= NOTE_COUNT) {
            /* 没有下一个音符，统一结束整段旋律。 */
            New_SequenceStop();
        } else {
            /* 仍有音符：立即播放，并重新建立NOTE的计时起点。 */
            Buzzer_Beep(notes[note_index]);
            sequence_start_ms = now;
            sequence_state = SEQUENCE_NOTE;
        }
    }
}

/*
 * 暂停正在进行的NOTE或GAP，并保存恢复所需的状态和已用时间。
 * 暂停只冻结播放过程，不清除音符下标，也不结束当前这次旋律。
 */
void New_SequencePause(void) {
    /* 记录暂停瞬间，用于计算当前状态已经经过的时间。 */
    u16 now = Timers_GetSystemMs();

    /* IDLE或已经PAUSED时不能重复执行暂停操作。 */
    if (sequence_state == SEQUENCE_NOTE || sequence_state == SEQUENCE_GAP) {
        /* 同时保存原状态和已用时间，恢复时才能计算正确的剩余时间。 */
        state_before_pause = sequence_state;
        paused_elapsed_ms = now - sequence_start_ms;

        /* 无论从NOTE还是GAP暂停，都确保蜂鸣器处于静音状态。 */
        Buzzer_Stop();
        sequence_state = SEQUENCE_PAUSED;
    }
}

/*
 * 从暂停点恢复NOTE或GAP，并继续暂停前尚未完成的剩余时间。
 * 恢复NOTE时重新启动当前音符，恢复GAP时继续保持静音。
 */
void New_SequenceResume(void) {
    /* 当前时间用于重新构造暂停前那个状态的开始时间。 */
    u16 now = Timers_GetSystemMs();

    /* 只有PAUSED状态允许恢复，其他状态调用时不做任何处理。 */
    if (sequence_state != SEQUENCE_PAUSED) {
        return;
    }

    /*
     * 用当前时间减去暂停前的已用时间，排除暂停期间经过的时间；
     * 原有的300 ms或50 ms判断随后只会等待尚未完成的部分。
     */
    sequence_state = state_before_pause;
    sequence_start_ms = now - paused_elapsed_ms;

    /* NOTE需要恢复声音；GAP本来就是静音，不能错误地开始播放。 */
    if (state_before_pause == SEQUENCE_NOTE) {
        Buzzer_Beep(notes[note_index]);
    } else if (state_before_pause == SEQUENCE_GAP) {
        Buzzer_Stop();
    }

    /* 恢复完成后清除本次暂停留下的临时记录。 */
    paused_elapsed_ms = 0;
    state_before_pause = SEQUENCE_IDLE;
}

/*
 * 更新三个按键的事件，并处理开始、暂停/继续和完全停止操作。
 * 每个事件在处理前先清零，保证一次有效按下只触发一次业务动作。
 */
void New_HandleKeys(void) {
    /* 公共Key模块扫描三个按键并生成经过10 ms消抖的一次性事件。 */
    Key_Scan();

    /* 读取时自动消费事件；K1只允许在IDLE时开始。 */
    if (Key_GetPressEvent(0) != 0) {
        if (sequence_state == SEQUENCE_IDLE) {
            New_SequenceStart();
        }
    }

    /* K2负责在正常播放和暂停状态之间切换。 */
    if (Key_GetPressEvent(1) != 0) {
        if (sequence_state == SEQUENCE_NOTE || sequence_state == SEQUENCE_GAP) {
            New_SequencePause();
        } else if (sequence_state == SEQUENCE_PAUSED) {
            New_SequenceResume();
        }
    }

    /* K3在NOTE、GAP或PAUSED中都执行完全停止；IDLE时无需处理。 */
    if (Key_GetPressEvent(2) != 0) {
        if (sequence_state != SEQUENCE_IDLE) {
            New_SequenceStop();
        }
    }
}

/*
 * 作为主循环入口，先处理按键，再按当前播放状态推进旋律状态机。
 * 本函数不包含阻塞延时，应由主循环持续、快速地重复调用。
 */
void New_SequenceTask(void) {
    /* 优先处理停止或暂停事件，避免本轮先错误推进一次播放状态。 */
    New_HandleKeys();

    /* PAUSED时仍属于本次播放，但New_PlaySequence()不会推进PAUSED状态。 */
    if (is_playing != 0) {
        New_PlaySequence();
    }
}



