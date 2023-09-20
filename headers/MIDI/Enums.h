#pragma once

namespace MIDI {

enum MIDI_CHUNK_TYPE { // augh goofy ahh literals 
	MIDI_CHUNK_TYPE_HEADER = 'MThd',
	MIDI_CHUNK_TYPE_TRACK = 'MTrk',
};

enum MIDI_FORMAT {
	MIDI_FORMAT_0 = 0,
	MIDI_FORMAT_1 = 1,
	MIDI_FORMAT_2 = 2,
};

enum MIDI_DIVISION_FORMAT {
	MIDI_DIVISION_FORMAT_TPQN = 0,
	MIDI_DIVISION_FORMAT_SMPTE = 1,
};

enum MIDI_SMPTE_FPS {
	MIDI_SMPTE_FPS_24 = -24,
	MIDI_SMPTE_FPS_25 = -25,
	MIDI_SMPTE_FPS_2997 = -29,
	MIDI_SMPTE_FPS_30 = -30,
};

enum MIDI_EVENT_STATUS {
	MIDI_EVENT_STATUS_NOTE_OFF = 8,
	MIDI_EVENT_STATUS_NOTE_ON = 9,
	MIDI_EVENT_STATUS_POLY_KEY_PRESSURE = 0xA,
	MIDI_EVENT_STATUS_CTRL_CHANGE = 0xB,
	MIDI_EVENT_STATUS_CHANNEL_MODE = 0xB,
	MIDI_EVENT_STATUS_PRGM_CHANGE = 0xC,
	MIDI_EVENT_STATUS_CHANNEL_PRESSURE = 0xD,
	MIDI_EVENT_STATUS_PITCH_WHEEL = 0xE,
	MIDI_EVENT_STATUS_SYSTEM = 0xF,
};

enum MIDI_EVENT_SYSCH {
	MIDI_EVENT_SYSCH_SYSTEM_EXLUSIVE = 0,
	MIDI_EVENT_SYSCH_SONG_POS = 2,
	MIDI_EVENT_SYSCH_SONG_SELECT = 3,
	MIDI_EVENT_SYSCH_TUNE = 6,
	MIDI_EVENT_SYSCH_EXCLUSIVE_END = 7,
	MIDI_EVENT_SYSCH_TIMING = 8,
	MIDI_EVENT_SYSCH_START = 0xA,
	MIDI_EVENT_SYSCH_RESUME = 0xB,
	MIDI_EVENT_SYSCH_STOP = 0xC,
	MIDI_EVENT_SYSCH_SENSING = 0xE,
	MIDI_EVENT_SYSCH_RESET = 0xF,
};

enum MIDI_EVENT_CTRL {
	MIDI_EVENT_CTRL_BANK_SELECT_1 = 0,
	MIDI_EVENT_CTRL_MOD_WHEEL_1,
	MIDI_EVENT_CTRL_BREATH_1,
	MIDI_EVENT_CTRL_FOOT_1,
	MIDI_EVENT_CTRL_GLIDE_TIME_1 = 5,
	MIDI_EVENT_CTRL_DATA_ENTRY_1,
	MIDI_EVENT_CTRL_CHANNEL_VOLUME_1,
	MIDI_EVENT_CTRL_BALANCE_1,
	MIDI_EVENT_CTRL_PAN_1 = 10,
	MIDI_EVENT_CTRL_EXPRESSION_1,
	MIDI_EVENT_CTRL_EFFECT1_1,
	MIDI_EVENT_CTRL_EFFECT2_1,
	MIDI_EVENT_CTRL_GENERAL1_1 = 16,
	MIDI_EVENT_CTRL_GENERAL2_1,
	MIDI_EVENT_CTRL_GENERAL3_1,
	MIDI_EVENT_CTRL_GENERAL4_,
	MIDI_EVENT_CTRL_BANK_SELECT_2 = 32,
	MIDI_EVENT_CTRL_MOD_WHEEL_2,
	MIDI_EVENT_CTRL_BREATH_2,
	MIDI_EVENT_CTRL_FOOT_2,
	MIDI_EVENT_CTRL_GLIDE_TIME_2 = 37,
	MIDI_EVENT_CTRL_DATA_ENTRY_2,
	MIDI_EVENT_CTRL_CHANNEL_VOLUME_2,
	MIDI_EVENT_CTRL_BALANCE_2,
	MIDI_EVENT_CTRL_PAN_2 = 42,
	MIDI_EVENT_CTRL_EXPRESSION_2,
	MIDI_EVENT_CTRL_EFFECT1_2,
	MIDI_EVENT_CTRL_EFFECT2_2,
	MIDI_EVENT_CTRL_GENERAL1_2 = 48,
	MIDI_EVENT_CTRL_GENERAL2_2,
	MIDI_EVENT_CTRL_GENERAL3_2,
	MIDI_EVENT_CTRL_GENERAL4_2,
	MIDI_EVENT_CTRL_DAMPER_SWITCH = 64,
	MIDI_EVENT_CTRL_GLIDE_SWITCH,
	MIDI_EVENT_CTRL_SUSTAIN_SWITCH,
	MIDI_EVENT_CTRL_SOFT_SWITCH,
	MIDI_EVENT_CTRL_LEGATO_FOOT_SWITCH,
	MIDI_EVENT_CTRL_HOLD2,
	MIDI_EVENT_CTRL_SOUND1,
	MIDI_EVENT_CTRL_SOUND2,
	MIDI_EVENT_CTRL_SOUND3,
	MIDI_EVENT_CTRL_SOUND4,
	MIDI_EVENT_CTRL_SOUND5,
	MIDI_EVENT_CTRL_SOUND6,
	MIDI_EVENT_CTRL_SOUND7,
	MIDI_EVENT_CTRL_SOUND8,
	MIDI_EVENT_CTRL_SOUND9,
	MIDI_EVENT_CTRL_SOUND10,
	MIDI_EVENT_CTRL_GENERAL5,
	MIDI_EVENT_CTRL_GENERAL6,
	MIDI_EVENT_CTRL_GENERAL7,
	MIDI_EVENT_CTRL_GENERAL8,
	MIDI_EVENT_CTRL_GLIDE,
	MIDI_EVENT_CTRL_EFFECT1_DEPTH = 91,
	MIDI_EVENT_CTRL_EFFECT2_DEPTH,
	MIDI_EVENT_CTRL_EFFECT3_DEPTH,
	MIDI_EVENT_CTRL_EFFECT4_DEPTH,
	MIDI_EVENT_CTRL_EFFECT5_DEPTH,

	MIDI_EVENT_CTRL_DATA_ENTRY_INC,
	MIDI_EVENT_CTRL_DATA_ENTRY_DEC,
	MIDI_EVENT_CTRL_NRPN_LSB,
	MIDI_EVENT_CTRL_NRPN_MSB,
	MIDI_EVENT_CTRL_RPN_LSB,
	MIDI_EVENT_CTRL_RPN_MSB,

	MIDI_EVENT_CTRL_ALL_SOUND_OFF = 120,
	MIDI_EVENT_CTRL_RESET_ALL,
	MIDI_EVENT_CTRL_LOCAL_SWITCH,
	MIDI_EVENT_CTRL_ALL_NOTES_OFF,
	MIDI_EVENT_CTRL_OMNI_OFF,
	MIDI_EVENT_CTRL_OMNI_ON,
	MIDI_EVENT_CTRL_POLY_SWITCH,
	MIDI_EVENT_CTRL_POLY_ON,
};

enum MIDI_META_EVENT {
	MIDI_META_EVENT_SEQUENCE_NUMBER = 0,
	MIDI_META_EVENT_TEXT = 1,
	MIDI_META_EVENT_COPYRIGHT = 2,
	MIDI_META_EVENT_TRACK_NAME = 3,
	MIDI_META_EVENT_INSTR_NAME = 4,
	MIDI_META_EVENT_LYRIC = 5,
	MIDI_META_EVENT_MARKER = 6,
	MIDI_META_EVENT_CUE = 7,
	MIDI_META_EVENT_CHANNEL = 0x20,
	MIDI_META_EVENT_EOT = 0x2F,
	MIDI_META_EVENT_TEMPO = 0x51,
	MIDI_META_EVENT_SMPTE_OFFSET = 0x54,
	MIDI_META_EVENT_TIME_SIG = 0x58,
	MIDI_META_EVENT_KEY_SIG = 0x59,
	MIDI_META_EVENT_SEQ_SPEC = 0x7F,
};

enum MIDI_NOTE {
	MIDI_NOTE_C0, MIDI_NOTE_CS0, MIDI_NOTE_Db0 = MIDI_NOTE_CS0, MIDI_NOTE_D0, MIDI_NOTE_DS0, MIDI_NOTE_Eb0 = MIDI_NOTE_DS0, MIDI_NOTE_E0, MIDI_NOTE_F0, MIDI_NOTE_FS0, 
	MIDI_NOTE_Gb0 = MIDI_NOTE_FS0, MIDI_NOTE_G0, MIDI_NOTE_GS0, MIDI_NOTE_Ab0 = MIDI_NOTE_GS0, MIDI_NOTE_A0, MIDI_NOTE_AS0, MIDI_NOTE_Bb0 = MIDI_NOTE_AS0, MIDI_NOTE_B0,
	MIDI_NOTE_C1, MIDI_NOTE_CS1, MIDI_NOTE_Db1 = MIDI_NOTE_CS1, MIDI_NOTE_D1, MIDI_NOTE_DS1, MIDI_NOTE_Eb1 = MIDI_NOTE_DS1, MIDI_NOTE_E1, MIDI_NOTE_F1, MIDI_NOTE_FS1, 
	MIDI_NOTE_Gb1 = MIDI_NOTE_FS1, MIDI_NOTE_G1, MIDI_NOTE_GS1, MIDI_NOTE_Ab1 = MIDI_NOTE_GS1, MIDI_NOTE_A1, MIDI_NOTE_AS1, MIDI_NOTE_Bb1 = MIDI_NOTE_AS1, MIDI_NOTE_B1,
	MIDI_NOTE_C2, MIDI_NOTE_CS2, MIDI_NOTE_Db2 = MIDI_NOTE_CS2, MIDI_NOTE_D2, MIDI_NOTE_DS2, MIDI_NOTE_Eb2 = MIDI_NOTE_DS2, MIDI_NOTE_E2, MIDI_NOTE_F2, MIDI_NOTE_FS2, 
	MIDI_NOTE_Gb2 = MIDI_NOTE_FS2, MIDI_NOTE_G2, MIDI_NOTE_GS2, MIDI_NOTE_Ab2 = MIDI_NOTE_GS2, MIDI_NOTE_A2, MIDI_NOTE_AS2, MIDI_NOTE_Bb2 = MIDI_NOTE_AS2, MIDI_NOTE_B2,
	MIDI_NOTE_C3, MIDI_NOTE_CS3, MIDI_NOTE_Db3 = MIDI_NOTE_CS3, MIDI_NOTE_D3, MIDI_NOTE_DS3, MIDI_NOTE_Eb3 = MIDI_NOTE_DS3, MIDI_NOTE_E3, MIDI_NOTE_F3, MIDI_NOTE_FS3, 
	MIDI_NOTE_Gb3 = MIDI_NOTE_FS3, MIDI_NOTE_G3, MIDI_NOTE_GS3, MIDI_NOTE_Ab3 = MIDI_NOTE_GS3, MIDI_NOTE_A3, MIDI_NOTE_AS3, MIDI_NOTE_Bb3 = MIDI_NOTE_AS3, MIDI_NOTE_B3,
	MIDI_NOTE_C4, MIDI_NOTE_CS4, MIDI_NOTE_Db4 = MIDI_NOTE_CS4, MIDI_NOTE_D4, MIDI_NOTE_DS4, MIDI_NOTE_Eb4 = MIDI_NOTE_DS4, MIDI_NOTE_E4, MIDI_NOTE_F4, MIDI_NOTE_FS4, 
	MIDI_NOTE_Gb4 = MIDI_NOTE_FS4, MIDI_NOTE_G4, MIDI_NOTE_GS4, MIDI_NOTE_Ab4 = MIDI_NOTE_GS4, MIDI_NOTE_A4, MIDI_NOTE_AS4, MIDI_NOTE_Bb4 = MIDI_NOTE_AS4, MIDI_NOTE_B4,
	MIDI_NOTE_C5, MIDI_NOTE_CS5, MIDI_NOTE_Db5 = MIDI_NOTE_CS5, MIDI_NOTE_D5, MIDI_NOTE_DS5, MIDI_NOTE_Eb5 = MIDI_NOTE_DS5, MIDI_NOTE_E5, MIDI_NOTE_F5, MIDI_NOTE_FS5, 
	MIDI_NOTE_Gb5 = MIDI_NOTE_FS5, MIDI_NOTE_G5, MIDI_NOTE_GS5, MIDI_NOTE_Ab5 = MIDI_NOTE_GS5, MIDI_NOTE_A5, MIDI_NOTE_AS5, MIDI_NOTE_Bb5 = MIDI_NOTE_AS5, MIDI_NOTE_B5,
	MIDI_NOTE_C6, MIDI_NOTE_CS6, MIDI_NOTE_Db6 = MIDI_NOTE_CS6, MIDI_NOTE_D6, MIDI_NOTE_DS6, MIDI_NOTE_Eb6 = MIDI_NOTE_DS6, MIDI_NOTE_E6, MIDI_NOTE_F6, MIDI_NOTE_FS6, 
	MIDI_NOTE_Gb6 = MIDI_NOTE_FS6, MIDI_NOTE_G6, MIDI_NOTE_GS6, MIDI_NOTE_Ab6 = MIDI_NOTE_GS6, MIDI_NOTE_A6, MIDI_NOTE_AS6, MIDI_NOTE_Bb6 = MIDI_NOTE_AS6, MIDI_NOTE_B6,
	MIDI_NOTE_C7, MIDI_NOTE_CS7, MIDI_NOTE_Db7 = MIDI_NOTE_CS7, MIDI_NOTE_D7, MIDI_NOTE_DS7, MIDI_NOTE_Eb7 = MIDI_NOTE_DS7, MIDI_NOTE_E7, MIDI_NOTE_F7, MIDI_NOTE_FS7, 
	MIDI_NOTE_Gb7 = MIDI_NOTE_FS7, MIDI_NOTE_G7, MIDI_NOTE_GS7, MIDI_NOTE_Ab7 = MIDI_NOTE_GS7, MIDI_NOTE_A7, MIDI_NOTE_AS7, MIDI_NOTE_Bb7 = MIDI_NOTE_AS7, MIDI_NOTE_B7,
	MIDI_NOTE_C8, MIDI_NOTE_CS8, MIDI_NOTE_Db8 = MIDI_NOTE_CS8, MIDI_NOTE_D8, MIDI_NOTE_DS8, MIDI_NOTE_Eb8 = MIDI_NOTE_DS8, MIDI_NOTE_E8, MIDI_NOTE_F8, MIDI_NOTE_FS8, 
	MIDI_NOTE_Gb8 = MIDI_NOTE_FS8, MIDI_NOTE_G8, MIDI_NOTE_GS8, MIDI_NOTE_Ab8 = MIDI_NOTE_GS8, MIDI_NOTE_A8, MIDI_NOTE_AS8, MIDI_NOTE_Bb8 = MIDI_NOTE_AS8, MIDI_NOTE_B8,
	MIDI_NOTE_C9, MIDI_NOTE_CS9, MIDI_NOTE_Db9 = MIDI_NOTE_CS9, MIDI_NOTE_D9, MIDI_NOTE_DS9, MIDI_NOTE_Eb9 = MIDI_NOTE_DS9, MIDI_NOTE_E9, MIDI_NOTE_F9, MIDI_NOTE_FS9, 
	MIDI_NOTE_Gb9 = MIDI_NOTE_FS9, MIDI_NOTE_G9, MIDI_NOTE_GS9, MIDI_NOTE_Ab9 = MIDI_NOTE_GS9, MIDI_NOTE_A9, MIDI_NOTE_AS9, MIDI_NOTE_Bb9 = MIDI_NOTE_AS9, MIDI_NOTE_B9,
	MIDI_NOTE_C10, MIDI_NOTE_CS10, MIDI_NOTE_Db10 = MIDI_NOTE_CS10, MIDI_NOTE_D10, MIDI_NOTE_DS10, MIDI_NOTE_Eb10 = MIDI_NOTE_DS10, MIDI_NOTE_E10, MIDI_NOTE_F10, MIDI_NOTE_FS10, 
	MIDI_NOTE_Gb10 = MIDI_NOTE_FS10, MIDI_NOTE_G10,
};



}