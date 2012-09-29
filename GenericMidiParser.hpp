/**
 * @file GenericMidiParser.hpp
 * @brief brief Generic, platform independent, Midi File Parser
 * @author SkyWodd
 * @version 2.0
 * @see http://skyduino.wordpress.com/
 *
 * @section intro_sec Introduction
 * This library is a generic, platform independent midi file parser.\n
 * \n
 * Please report bug to <skywodd at gmail.com>
 *
 * @section licence_sec Licence
 *  This program is free software: you can redistribute it and/or modify\n
 *  it under the terms of the GNU General Public License as published by\n
 *  the Free Software Foundation, either version 3 of the License, or\n
 *  (at your option) any later version.\n
 * \n
 *  This program is distributed in the hope that it will be useful,\n
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of\n
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n
 *  GNU General Public License for more details.\n
 * \n
 *  You should have received a copy of the GNU General Public License\n
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.\n
 *
 * @section changelog_sec Changelog history
 * - 11/04/2012 : Initial commit
 * - 16/04/2012 : First release (under GNU GPL V3 licence)
 * - 17/04/2012 : Add simultanous multiple tracks support
 *              : Add debug macro
 *
 * @section other_sec Others notes and compatibility warning
 * This version can handle simultaneous tracks parsing (limit of simutaneous tracks is defined in header file).
 */

#ifndef GENERICMIDIPARSER_HPP_
#define GENERICMIDIPARSER_HPP_

#include <stdint.h>

/**
 * Debug ouput
 */
//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DEBUG(format, ...) printf("DBG: " format "\n", ##__VA_ARGS__)
#include <stdio.h>
#else
#define DEBUG(format, ...) {}
#endif

/**
 * Define (if not allready) the maximum number of midi tracks to process
 */
#ifndef MAX_TRACKS_NUMBERS
#define MAX_TRACKS_NUMBERS 12
#endif

/**
 * GenericMidiParser class
 */
class GenericMidiParser {

private:
	/**
	 * Midi file header structure
	 */
	typedef struct {
		uint32_t headerSize; // Big endian
		uint16_t formatType; // Big endian
		uint16_t numberOfTracks; // Big endian
		int16_t timeDivision;
	} MidiHeader;

	/**
	 * Midi track header structure
	 */
	typedef struct {
		uint32_t trackPointer;
		uint32_t trackSize;
		uint32_t waitTime;
		uint8_t done;
	} TrackHeader;

public:

	/**
	 * Enumeration of possible error codes
	 */
	enum {
		NO_ERROR,
		BAD_FILE_HEADER,
		BAD_TRACK_HEADER,
		BAD_META_EVENT,
		BAD_FILE_STRUCT,
		NO_MULTIPLE_SONG_SUPPORT,
		NO_SMPTE_SUPPORT,
	};

	/**
	 * Enumeration of midi file types
	 */
	enum {
		SINGLE_TRACK_FILE, MULTIPLE_TRACK_FILE, MULTILPLE_SONG_FILE,
	};

	/**
	 * Enumeration of meta text events
	 */
	enum {
		META_TEXT = 1,
		META_COPYRIGHT,
		META_TRACK_NAME,
		META_INSTRUMENT,
		META_LYRICS,
		META_MARKER,
		META_CUE_POINT,
		META_CHANNEL,
		META_SEQUENCER,
		META_SYSEX
	};

private:
	/* Misc. */
	MidiHeader header;
	TrackHeader tracks[MAX_TRACKS_NUMBERS];
	uint8_t current_track_number, track_finished;

	volatile uint8_t paused;
	uint8_t errno;
	uint32_t tempo;

	/* Low-level functions */
	uint8_t (*file_read_fnct)(void);
	void (*file_fseek_fnct)(uint32_t address);
	uint32_t (*file_ftell_fnct)(void);
	uint8_t (*file_eof_fnct)(void);
	void (*us_delay_fnct)(uint32_t us);
	void (*assert_error_callback)(uint8_t errorCode);

	/* Callback function */
	void (*note_on_callback)(uint8_t channel, uint8_t key, uint8_t velocity);
	void (*note_off_callback)(uint8_t channel, uint8_t key, uint8_t velocity);
	void (*key_after_touch_callback)(uint8_t channel, uint8_t key,
			uint8_t pressure);
	void (*control_change_callback)(uint8_t channel, uint8_t controller,
			uint8_t data);
	void (*patch_change_callback)(uint8_t channel, uint8_t instrument);
	void (*channel_after_touch_callback)(uint8_t channel, uint8_t pressure);
	void (*pitch_bend_callback)(uint8_t channel, uint16_t bend);
	void (*meta_callback)(uint8_t metaType, uint8_t dataLength);
	void (*meta_onChannel_prefix)(uint8_t channel);
	void (*meta_onPort_prefix)(uint8_t channel);
	void (*time_signature_callback)(uint8_t numerator, uint8_t denominator,
			uint8_t metronomeTick, uint8_t note32NdNumber);
	void (*key_signature_callback)(uint8_t sharpsFlats, uint8_t majorMinor);

	/* Usefull functions */
	uint8_t processHeader();
	uint8_t processTrack();
	void processTime();
	uint8_t processEvent();
	uint8_t processMeta(uint8_t cmd);
	uint32_t readVarLenValue();
	uint8_t minTime(uint32_t *min);

public:

	GenericMidiParser(uint8_t (*file_read_fnct)(void),
			void (*file_fseek_fnct)(uint32_t address),
			uint32_t (*file_ftell_fnct)(void), uint8_t (*file_eof_fnct)(void),
			void (*us_delay_fnct)(uint32_t us),
			void (*assert_error_callback)(uint8_t errorCode));

	/* Callback setter functions */

	void setNoteOnCallback(
			void (*note_on_callback)(uint8_t channel, uint8_t key,
					uint8_t velocity));

	void setNoteOffCallback(
			void (*note_off_callback)(uint8_t channel, uint8_t key,
					uint8_t velocity));

	void setKeyAfterTouchCallback(
			void (*key_after_touch_callback)(uint8_t channel, uint8_t key,
					uint8_t pressure));

	void setControlChangeCallback(
			void (*control_change_callback)(uint8_t channel, uint8_t controller,
					uint8_t data));

	void setPatchChangeCallback(
			void (*patch_change_callback)(uint8_t channel, uint8_t instrument));

	void setChannelAfterTouchCallback(
			void (*channel_after_touch_callback)(uint8_t channel,
					uint8_t pressure));

	void setPitchBendCallback(
			void (*pitch_bend_callback)(uint8_t channel, uint16_t bend));

	void setMetaCallback(
			void (*meta_callback)(uint8_t metaType, uint8_t dataLength));

	void setMetaOnChannelCallback(
			void (*meta_onChannel_prefix)(uint8_t channel));

	void setMetaOnPortCallback(void (*meta_onPort_prefix)(uint8_t channel));

	void setTimeSignatureCallback(
			void (*time_signature_callback)(uint8_t numerator,
					uint8_t denominator, uint8_t metronomeTick,
					uint8_t note32NdNumber));

	void setKeySignatureCallback(
			void (*key_signature_callback)(uint8_t sharpsFlats,
					uint8_t majorMinor));

	/* General functions */

	uint32_t readByte(); // uint32_t to avoid bitwise overflow error
	void readBytes(uint8_t* buf, uint8_t len);
	void dropBytes(uint8_t len);

	/* Control functions */

	void play();

	void pause();

	void resume();

	void stop();

	/* Getter Setter functions */

	uint8_t getErrno() const;

	uint32_t getTempo() const;

	void setTempo(uint32_t tempo);
};

#endif /* GENERICMIDIPARSER_HPP_ */
