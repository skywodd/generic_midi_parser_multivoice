/* Includes */
#include <stdio.h>
#include "GenericMidiParser.hpp"

FILE *fi;

/* Low-level functions */
uint8_t file_read_fnct(void) {
	uint8_t buf;
	fread(&buf, 1, 1, fi);
	//printf("READ : %x\n", buf);
	return buf;
}

uint8_t file_eof_fnct(void) {
	return feof(fi);
}

void file_fseek_fnct(uint32_t address) {
	fseek(fi, address, SEEK_SET);
}

uint32_t file_ftell_fnct(void) {
	return ftell(fi);
}

void us_delay_fnct(uint32_t us) {
	printf("Delay %u ms\n", us / 1000);
}

void assert_error_callback(uint8_t errorCode) {
	printf("Error : %d\n", errorCode);
}

GenericMidiParser midi(file_read_fnct, file_fseek_fnct, file_ftell_fnct,
		file_eof_fnct, us_delay_fnct, assert_error_callback);

/* Callback function */
void note_on_callback(uint8_t channel, uint8_t key, uint8_t velocity) {
	printf("Note on : channel %d, key %d, velocity %d\n", channel, key,
			velocity);
}

void note_off_callback(uint8_t channel, uint8_t key, uint8_t velocity) {
	printf("Note off : channel %d, key %d, velocity %d\n", channel, key,
			velocity);
}

void key_after_touch_callback(uint8_t channel, uint8_t key, uint8_t pressure) {
	printf("Key after touch : channel %d, key %d, pressure %d\n", channel, key,
			pressure);
}

void control_change_callback(uint8_t channel, uint8_t controller,
		uint8_t data) {
	printf("Control change : channel %d, controller %d, data %d\n", channel,
			controller, data);
}

void patch_change_callback(uint8_t channel, uint8_t instrument) {
	printf("Patch change : channel %d, instrument %d\n", channel, instrument);
}

void channel_after_touch_callback(uint8_t channel, uint8_t pressure) {
	printf("channel after touch : channel %d, pressure %d\n", channel,
			pressure);
}

void pitch_bend_callback(uint8_t channel, uint16_t bend) {
	printf("Pitch bend : channel %d, bend %d\n", channel, bend);
}

void meta_callback(uint8_t metaType, uint8_t dataLength) {
	uint8_t* buf = new uint8_t[dataLength];
	printf("Meta event : type %d, length %d\n", metaType, dataLength);
	midi.readBytes(buf, dataLength);
	for (int i = 0; i < dataLength; i++)
		printf("%c", (buf[i] >= ' ') ? buf[i] : ' ');
	puts("");
	delete[] buf;
}

void meta_onChannel_prefix(uint8_t channel) {
	printf("Meta onChannel prefix : %d\n", channel);
}

void meta_onPort_prefix(uint8_t channel) {
	printf("Meta onPort prefix : %d\n", channel);
}

int main(void) {

	fi = fopen("test.mid", "rb");
	if (fi == NULL) {
		puts("Impossible d'ouvrir le fichier de test !");
		return 1;
	}

	midi.setNoteOnCallback(note_on_callback);
	midi.setNoteOffCallback(note_off_callback);
	midi.setKeyAfterTouchCallback(key_after_touch_callback);
	midi.setControlChangeCallback(control_change_callback);
	midi.setPatchChangeCallback(patch_change_callback);
	midi.setChannelAfterTouchCallback(channel_after_touch_callback);
	midi.setPitchBendCallback(pitch_bend_callback);
	midi.setMetaCallback(meta_callback);
	midi.setMetaOnChannelCallback(meta_onChannel_prefix);
	midi.setMetaOnPortCallback(meta_onPort_prefix);

	midi.play();

	return 0;
}
