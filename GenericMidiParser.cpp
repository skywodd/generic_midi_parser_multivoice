/*
 * See header file for details
 *
 *  This program is free software: you can redistribute it and/or modify\n
 *  it under the terms of the GNU General Public License as published by\n
 *  the Free Software Foundation, either version 3 of the License, or\n
 *  (at your option) any later version.\n
 * 
 *  This program is distributed in the hope that it will be useful,\n
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of\n
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n
 *  GNU General Public License for more details.\n
 * 
 *  You should have received a copy of the GNU General Public License\n
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.\n
 */
/* Includes */
#include "GenericMidiParser.hpp"

GenericMidiParser::GenericMidiParser(uint8_t (*file_read_fnct)(void),
		void (*file_fseek_fnct)(uint32_t address),
		uint32_t (*file_ftell_fnct)(void), uint8_t (*file_eof_fnct)(void),
		void (*us_delay_fnct)(uint32_t us),
		void (*assert_error_callback)(uint8_t errorCode)) :
		file_read_fnct(file_read_fnct), file_fseek_fnct(file_fseek_fnct), file_ftell_fnct(
				file_ftell_fnct), file_eof_fnct(file_eof_fnct), us_delay_fnct(
				us_delay_fnct), assert_error_callback(assert_error_callback), note_on_callback(
				0), note_off_callback(0), key_after_touch_callback(0), control_change_callback(
				0), patch_change_callback(0), channel_after_touch_callback(0), pitch_bend_callback(
				0), meta_callback(0), meta_onChannel_prefix(0), meta_onPort_prefix(
				0), time_signature_callback(0), key_signature_callback(0) {

}

uint32_t GenericMidiParser::readByte() {
	tracks[current_track_number].trackSize--;
	tracks[current_track_number].trackPointer++;
	return file_read_fnct();
}

void GenericMidiParser::readBytes(uint8_t* buf, uint8_t len) {
	tracks[current_track_number].trackSize -= len;
	tracks[current_track_number].trackPointer += len;
	for (uint8_t i = 0; i < len; i++)
		buf[i] = file_read_fnct();
}

void GenericMidiParser::dropBytes(uint8_t len) {
	//tracks[current_track_number].trackSize -= len;
	tracks[current_track_number].trackPointer += len;
}

uint32_t GenericMidiParser::readVarLenValue() {
	uint32_t value = 0;
	uint8_t c;
	if ((value = readByte()) & 0x80) {
		value &= 0x7F;
		do {
			value = (value << 7) + ((c = readByte()) & 0x7F);
		} while (c & 0x80);
	}
	return value;
}

void GenericMidiParser::setNoteOnCallback(
		void (*note_on_callback)(uint8_t channel, uint8_t key,
				uint8_t velocity)) {
	this->note_on_callback = note_on_callback;
}

void GenericMidiParser::setNoteOffCallback(
		void (*note_off_callback)(uint8_t channel, uint8_t key,
				uint8_t velocity)) {
	this->note_off_callback = note_off_callback;
}

void GenericMidiParser::setKeyAfterTouchCallback(
		void (*key_after_touch_callback)(uint8_t channel, uint8_t key,
				uint8_t pressure)) {
	this->key_after_touch_callback = key_after_touch_callback;
}

void GenericMidiParser::setControlChangeCallback(
		void (*control_change_callback)(uint8_t channel, uint8_t controller,
				uint8_t data)) {
	this->control_change_callback = control_change_callback;
}

void GenericMidiParser::setPatchChangeCallback(
		void (*patch_change_callback)(uint8_t channel, uint8_t instrument)) {
	this->patch_change_callback = patch_change_callback;
}

void GenericMidiParser::setChannelAfterTouchCallback(
		void (*channel_after_touch_callback)(uint8_t channel,
				uint8_t pressure)) {
	this->channel_after_touch_callback = channel_after_touch_callback;
}

void GenericMidiParser::setPitchBendCallback(
		void (*pitch_bend_callback)(uint8_t channel, uint16_t bend)) {
	this->pitch_bend_callback = pitch_bend_callback;
}

void GenericMidiParser::setMetaCallback(
		void (*meta_callback)(uint8_t metaType, uint8_t dataLength)) {
	this->meta_callback = meta_callback;
}

void GenericMidiParser::setMetaOnChannelCallback(
		void (*meta_onChannel_prefix)(uint8_t channel)) {
	this->meta_onChannel_prefix = meta_onChannel_prefix;
}

void GenericMidiParser::setMetaOnPortCallback(
		void (*meta_onPort_prefix)(uint8_t channel)) {
	this->meta_onPort_prefix = meta_onPort_prefix;
}

void GenericMidiParser::setTimeSignatureCallback(
		void (*time_signature_callback)(uint8_t numerator, uint8_t denominator,
				uint8_t metronomeTick, uint8_t note32NdNumber)) {
	this->time_signature_callback = time_signature_callback;
}

void GenericMidiParser::setKeySignatureCallback(
		void (*key_signature_callback)(uint8_t sharpsFlats,
				uint8_t majorMinor)) {
	this->key_signature_callback = key_signature_callback;
}

uint8_t GenericMidiParser::processHeader() {
	DEBUG("Beginning header parsing ...");

	char MThd[4];
	readBytes((uint8_t*) MThd, 4);
	DEBUG("Header : %x %x %x %x", MThd[0], MThd[1], MThd[2], MThd[3]);

	header.headerSize = (readByte() << 24) | (readByte() << 16)
			| (readByte() << 8) | readByte();
	DEBUG("HeaderSize : %d", header.headerSize);

	header.formatType = (readByte() << 8) | readByte();
	DEBUG("Format type : %d", header.formatType);

	header.numberOfTracks = (readByte() << 8) | readByte();
	DEBUG("Number of track : %d", header.numberOfTracks);

	if (header.numberOfTracks > MAX_TRACKS_NUMBERS) {
		DEBUG("Number of track stripped down to %d", MAX_TRACKS_NUMBERS);
		header.numberOfTracks = MAX_TRACKS_NUMBERS;
	}

	header.timeDivision = (readByte() << 8) | readByte();
	DEBUG("Time division : %d", header.timeDivision);

	if (MThd[0] != 0x4D || MThd[1] != 0x54 || MThd[2] != 0x68
			|| MThd[3] != 0x64) {
		DEBUG("Header check: ERROR");
		return BAD_FILE_HEADER;
	}
	DEBUG("Header check: PASS");

	if (header.headerSize != 0x06) {
		DEBUG("Header size check: ERROR");
		return BAD_FILE_HEADER;
	}
	DEBUG("Header size check: PASS");

	if (header.formatType == MULTILPLE_SONG_FILE) {
		DEBUG("Fileformat check: ERROR");
		return NO_MULTIPLE_SONG_SUPPORT;
	}
	DEBUG("Fileformat check: PASS");

	if (header.timeDivision < 0) {
		DEBUG("Time division check: ERROR");
		return NO_SMPTE_SUPPORT; // TODO
	}
	DEBUG("Time division check: PASS");

	tempo = 500000; // Default tempo
	DEBUG("Tempo: %d", tempo);

	if (file_eof_fnct()) {
		DEBUG("File struct check: ERROR");
		return BAD_FILE_STRUCT;
	}
	DEBUG("File struct check: PASS");

	DEBUG("Header parsing done !");
	return NO_ERROR;
}

uint8_t GenericMidiParser::processTrack() {
	DEBUG("Beginning parsing track %d ...", current_track_number);

	char MTrk[4];
	readBytes((uint8_t*) MTrk, 4);
	DEBUG("Header : %x %x %x %x", MTrk[0], MTrk[1], MTrk[2], MTrk[3]);

	tracks[current_track_number].trackSize = (readByte() << 24)
			| (readByte() << 16) | (readByte() << 8) | readByte();
	DEBUG("TrackSize : %d", tracks[current_track_number].trackSize);

	if (MTrk[0] != 0x4D || MTrk[1] != 0x54 || MTrk[2] != 0x72
			|| MTrk[3] != 0x6B) {
		DEBUG("Header check: ERROR");
		return BAD_TRACK_HEADER;
	}
	DEBUG("Header check: PASS");

	tracks[current_track_number].trackPointer = file_ftell_fnct();
	DEBUG("Track pointer : %x", tracks[current_track_number].trackPointer);

	tracks[current_track_number].done = false;

	DEBUG("Track parsing done !");
	return NO_ERROR;
}

void GenericMidiParser::processTime() {
	file_fseek_fnct(tracks[current_track_number].trackPointer);
	DEBUG("Process DeltaTime from track %d", current_track_number);

	uint32_t deltaTime = readVarLenValue();
	DEBUG("Delta time: %d", deltaTime);

	DEBUG("TimeDivision: %d", header.timeDivision);
	DEBUG("Tempo: %d", tempo);
	DEBUG("DeltaTime: %d", deltaTime);
	tracks[current_track_number].waitTime = deltaTime
			* ((float) tempo / header.timeDivision);
}

uint8_t GenericMidiParser::processEvent() {
	file_fseek_fnct(tracks[current_track_number].trackPointer);
	DEBUG("Process Event from track %d", current_track_number);

	uint8_t cmd = readByte();
	uint8_t nybble = (cmd & 0xF0) >> 4;
	static uint8_t channel = 0;

	DEBUG("Command: %x", cmd);
	DEBUG("Nybble: %x", nybble);

	if (cmd < 0x80) { // Runnning status
		DEBUG("Event: Runnning status");
		DEBUG("Channel: %d", channel);

		uint8_t velocity = readByte();

		if (velocity > 0) {
			if (note_on_callback)
				note_on_callback(channel, cmd, velocity);
		} else {
			if (note_off_callback)
				note_off_callback(channel, cmd, velocity);
		}
	}

	channel = cmd & 0x0F;
	DEBUG("Channel: %d", channel);

	switch (nybble) {
	case 0x08: // note off
		DEBUG("Event: Note Off");
		if (note_off_callback)
			note_off_callback(channel, readByte(), readByte());
		else
			dropBytes(2);
		break;

	case 0x09: // note on
		DEBUG("Event: Note On");
		if (note_on_callback)
			note_on_callback(channel, readByte(), readByte());
		else
			dropBytes(2);
		break;

	case 0x0A: // key after-touch
		DEBUG("Event: Key after touch");
		if (key_after_touch_callback)
			key_after_touch_callback(channel, readByte(), readByte());
		else
			dropBytes(2);
		break;

	case 0x0B: // control change
		DEBUG("Event: Control change");
		if (control_change_callback)
			control_change_callback(channel, readByte(), readByte());
		else
			dropBytes(2);
		break;

	case 0x0C: // program change
		DEBUG("Event: Program change");
		if (patch_change_callback)
			patch_change_callback(channel, readByte());
		else
			readByte();
		break;

	case 0x0D: // channel after touch
		DEBUG("Event: Channel after touch");
		if (channel_after_touch_callback)
			channel_after_touch_callback(channel, readByte());
		else
			readByte();
		break;

	case 0x0E: // pitch wheel change
		DEBUG("Event: Pitch wheel change");
		if (pitch_bend_callback)
			pitch_bend_callback(channel, readByte());
		else
			readByte();
		break;

	case 0x0F: // meta
		DEBUG("Event: Meta");
		if ((errno = processMeta(cmd)))
			return errno;
		break;
	}

	return NO_ERROR;
}

uint8_t GenericMidiParser::processMeta(uint8_t cmd) {
	if (cmd == 0xFF) {
		DEBUG("Meta type: normal");

		uint8_t metaCmd = readByte();
		uint32_t dataLen = readVarLenValue();
		DEBUG("Meta Command: %x", metaCmd);
		DEBUG("Meta length: %d", dataLen);

		switch (metaCmd) {
		case 0x00: // Set track's sequence number
			DEBUG("Meta Event: set track number");
			if (dataLen != 0x02)
				return BAD_META_EVENT;
			//current_track_number = (readByte() << 8) | readByte(); // TODO
			break;

		case 0x01: // Text event- any text you want.
		case 0x02: // Same as text event, but used for copyright info.
		case 0x03: // Sequence or Track name
		case 0x04: // Track instrument name
		case 0x05: // Lyric
		case 0x06: // Marker
		case 0x07: // Cue point
			DEBUG("Meta Event: text or similar");
			if (dataLen == 0)
				return BAD_META_EVENT;
			if (meta_callback)
				meta_callback(metaCmd, dataLen);
			else
				dropBytes(dataLen);
			break;

		case 0x20: // Midi Channel Prefix
			DEBUG("Meta Event: Channel prefix");
			if (dataLen != 1)
				return BAD_META_EVENT;
			if (meta_onChannel_prefix)
				meta_onChannel_prefix(readByte());
			else
				readByte();
			break;

		case 0x21: // Midi Port Prefix
			DEBUG("Meta Event: Port prefix");
			if (dataLen != 1)
				return BAD_META_EVENT;
			if (meta_onPort_prefix)
				meta_onPort_prefix(readByte());
			else
				readByte();
			break;

		case 0x2F: // This event MUST come at the end of each tracks
			DEBUG("Meta Event: End of track");
			if (dataLen != 0x00)
				return BAD_META_EVENT;
			tracks[current_track_number].done = true;
			track_finished++;
			DEBUG("End of track %d", current_track_number);
			break;

		case 0x51: // Set tempoSet tempo
			DEBUG("Meta Event: Set tempo");
			if (dataLen != 0x03)
				return BAD_META_EVENT;
			tempo = (readByte() << 16) | (readByte() << 8) | readByte();
			DEBUG("Tempo: %d", tempo);
			break;

		case 0x54: // SMTPE Offset TODO
			DEBUG("Meta Event: SMTPE offset");
			if (dataLen != 0x05)
				return BAD_META_EVENT;
			dropBytes(5);
			/*uint8_t hours = readByte();
			 uint8_t minutes = readByte();
			 uint8_t seconds = readByte();
			 uint8_t frames = readByte();
			 uint8_t fractional = readByte();*/
			// SMTPE not implemented
			break;

		case 0x58: // Time Signature
			DEBUG("Meta Event: Time signature");
			if (dataLen != 0x04)
				return BAD_META_EVENT;
			if (time_signature_callback)
				time_signature_callback(readByte(), readByte(), readByte(),
						readByte());
			else
				dropBytes(4);
			break;

		case 0x59: // Key signature
			DEBUG("Meta Event: Key signature");
			if (dataLen != 0x02)
				return BAD_META_EVENT;
			if (key_signature_callback)
				key_signature_callback(readByte(), readByte());
			else
				dropBytes(2);
			break;

		case 0x7F: // Sequencer specific information
			DEBUG("Meta Event: Sequencer specific");
			if (dataLen == 0)
				return BAD_META_EVENT;
			if (meta_callback)
				meta_callback(META_SEQUENCER, dataLen);
			else
				dropBytes(dataLen);
			break;
		}

	} else {
		DEBUG("Meta type: sysex");

		switch (cmd) {
		case 0xF8: // Timing Clock Request
			DEBUG("Meta Event: Timing clock request");
			break;

		case 0xFA: // Start Sequence
			DEBUG("Meta Event: Start sequence");
			paused = false;
			break;

		case 0xFB: // Continue Stopped Sequence
			DEBUG("Meta Event: Resume sequence");
			paused = false;
			break;

		case 0xFC: // Stop Sequence
			DEBUG("Meta Event: Stop sequence");
			paused = true;
			break;

		case 0xF0: // sysex event
		case 0xF7: // sysex event
			DEBUG("Meta Event: Sysex message");
			uint32_t dataLen = readVarLenValue();
			DEBUG("Sysex length: %d", dataLen);

			if (dataLen == 0)
				return BAD_META_EVENT;
			if (meta_callback)
				meta_callback(META_SYSEX, dataLen);
			else
				dropBytes(dataLen);
			break;
		}
	}

	return NO_ERROR;
}

uint8_t GenericMidiParser::minTime(uint32_t *min) {
	uint8_t i, found = false;

	for (i = 0; i < header.numberOfTracks; i++)
		if (!tracks[i].done) {
			*min = tracks[i].waitTime;
			found = true;
		}

	for (i = 0; i < header.numberOfTracks; i++)
		if (!tracks[i].done)
			if (tracks[i].waitTime < *min)
				*min = tracks[i].waitTime;

	return found;
}

void GenericMidiParser::play() {
	file_fseek_fnct(0);

	errno = processHeader();
	if (errno) {
		assert_error_callback(errno);
		return;
	}

	for (current_track_number = 0; current_track_number < header.numberOfTracks;
			current_track_number++) {
		errno = processTrack();
		if (errno) {
			assert_error_callback(errno);
			return;
		}
		file_fseek_fnct(
				tracks[current_track_number].trackPointer
						+ tracks[current_track_number].trackSize);
	}

	DEBUG("Start playing ...");
	paused = false;
	track_finished = 0;

	for (current_track_number = 0; current_track_number < header.numberOfTracks;
			current_track_number++)
		processTime();

	uint32_t minWaitTime = 0;
	while (track_finished != header.numberOfTracks && !file_eof_fnct()) {

		while (paused) {
		}

		minTime(&minWaitTime);
		us_delay_fnct(minWaitTime);

		for (current_track_number = 0;
				current_track_number < header.numberOfTracks;
				current_track_number++)
			if (tracks[current_track_number].waitTime > minWaitTime)
				tracks[current_track_number].waitTime -= minWaitTime;
			else if (!tracks[current_track_number].done) {
				errno = processEvent();
				if (errno) {
					assert_error_callback(errno);
					return;
				}
				processTime();
			}
	}

	DEBUG("End of midi song ...");
}

void GenericMidiParser::pause() {
	paused = true;
}

void GenericMidiParser::resume() {
	paused = false;
}

void GenericMidiParser::stop() {
	track_finished = header.numberOfTracks;
}

uint8_t GenericMidiParser::getErrno() const {
	return errno;
}

uint32_t GenericMidiParser::getTempo() const {
	return tempo;
}

void GenericMidiParser::setTempo(uint32_t tempo) {
	this->tempo = tempo;
}
