// File name: WP_packet_support
//
// 23 Sep 2016 .. moved get_packet and send_packet into separate file .. KM
// 12 Oct 2016 .. added/modified code and added some comments
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#define WP_SOH 0xA5
#define WP_SOH_TRUE 0x00
#define WP_SOH_DATA 0x01
#define WP_SOH_SYNC 0x02

// get_packet state definitions

#define READ_IDLE 1
#define FOUND_SID 2
#define READ_SIZE 3
#define READ_DATA 4

#ifdef vc5505
#define uint8_t uint16_t
#endif
#ifdef c5515
#define uint8_t uint16_t
#endif

uint16_t	ReceiveFromRemote(uint8_t *ch);	// read characters from remote 
void		SendToRemote(uint8_t value); 	// write characters to remote
void 		mySleep(uint16_t value);		// used if multiple threads present

static uint8_t	*buffer_ptr;
static uint16_t read_state;
static uint16_t read_SID, read_size;
static uint16_t drchar_ctr;

// GetPacket from remote device.  
//
// 		packet structure:
// 			SOH 				start of header character
//			SOH_ID |(SID<<3)    soh confirmation and the stream ID
//			size                size of the data area in bytes < 256
//			data values...      the data values
//
// 		updates the caller's stream id and length values
//		return 0 if all went well

uint16_t GetPacket(uint16_t *sid, uint8_t *packet_buffer, uint16_t *len)
{
    uint8_t value, sw;
    
	read_state = READ_IDLE;
	sw = 0;

	while(1) {

		while (ReceiveFromRemote(&value) == 0) mySleep(1);	// wait for an input value
		if (value == WP_SOH) {					// WP_SOH characters are handled separately
			while (ReceiveFromRemote(&value) == 0) mySleep(1);	// get the immediately following character
			switch(value&0x07) {				// have variants on what to do
			case WP_SOH_TRUE:					// have actual WP_SOH	
				read_SID = (value >> 3)&0x1F;
				read_state = FOUND_SID;			// starts the reading of the packet
				break;
			case WP_SOH_DATA:					// have a have a SOH data value
				value = WP_SOH;					// which is a WP_SOH character
				break;
			case WP_SOH_SYNC:					// have a sync .. not presently used at this end
				read_state = READ_IDLE;
				break;
			default:							// have a bogus SOH variant
				printf("WP_SOH problem\n");
				while(1);
				break;
			}
		}

		switch(read_state) {				// a simple state machine
		case READ_IDLE:
			break;
        case FOUND_SID:						// found SOH and SID value
            read_state = READ_SIZE;
            break;
		case READ_SIZE:						// now have the size of data area
			read_size = value;
			drchar_ctr = 0;
			buffer_ptr = packet_buffer;
			read_state = READ_DATA;			// read in the packet
			break;
		case READ_DATA:						// place data values into buffer
			*buffer_ptr++ = value; 			// need to check bounds!!! todo!!
			drchar_ctr++;					// count this character
			if (drchar_ctr == read_size) {	// check if packet is complete
				read_state = READ_IDLE; 	// if so, reset packet support
				sw = 1;
			} // to break out of the loop
            break;
		default:							// should not be possible
			while(1);						// hang here
		} // end of read_state switch

		if (sw != 0) break;		// out of the while(1) when data is complete
	} // end of while(1)
	*sid = read_SID;
	*len = drchar_ctr;
	return 1;						// return size of data portion of packet
}

void SendProtectedSOH(uint8_t value)
{	
	SendToRemote(value);
	if (value == WP_SOH) SendToRemote(WP_SOH_DATA);
}

int16_t SendPacket(uint16_t sid, uint8_t *ptr, uint16_t len)
{
	uint8_t utemp;
	uint8_t data_ctr;

	SendToRemote(WP_SOH);			// start of header
	SendProtectedSOH((sid<<3)| WP_SOH_TRUE);
	SendProtectedSOH(len);			// amount of data to follow
	for (data_ctr=0; data_ctr<len; data_ctr++) {
		utemp = *ptr++ & 0x00FF;
		SendProtectedSOH(utemp);
	} // end of packet sending
	return 1;		
}

// Tell the remote to do a program (re)start.

void RestartRemote(void)
{
	SendToRemote(WP_SOH);
	SendToRemote(WP_SOH_SYNC);
}

//*************************************************
//
//	Simple support to build and send packets.
//  Primarily meant for use by WindowPlot support.
//  WindowPlot's data stream is sent to TX_Put.
//

#define MAX_SID_BUFFER_SIZE 254

struct StreamDCB {		// definition of stream data container block
	int16_t sid;
	int16_t counter;
	int16_t max_size;
	uint8_t buffer[MAX_SID_BUFFER_SIZE];
};

void Send_Flush(struct StreamDCB *ptr)
{
	if (ptr->counter != 0) {
		SendPacket(ptr->sid, ptr->buffer, ptr->counter);
		ptr->counter = 0;
	}
}

void Send_Buffered(struct StreamDCB *ptr, uint8_t value)
{
	ptr->buffer[ptr->counter++] = value;
	if (ptr->counter >= ptr->max_size) {
		Send_Flush(ptr);
	} // end packet sent
}

void sidBuffer_Initialize(uint16_t sid, struct StreamDCB *ptr)
{
	ptr->sid = sid;
	ptr->counter = 0;
	ptr->max_size = MAX_SID_BUFFER_SIZE;
}

//********************************************************************

// TX_Put is used only by WindowPlot.
// sid 3 is for use only by WindowPlot.

struct StreamDCB StreamDCB_WP = { 3, 0, MAX_SID_BUFFER_SIZE, {0} };	// used only by WindowPlot

void TX_Put(uint16_t value)		// used only for WindowPlot
{
	Send_Buffered(&StreamDCB_WP, value >> 8);
	Send_Buffered(&StreamDCB_WP, value & 0x00FF);
}

