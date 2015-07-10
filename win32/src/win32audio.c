#include "win32audio.h"
#include "../../common/src/oocommon.h"

#include <math.h> // TODO remove

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(directSoundCreate_function);


#define OOWIN32_AUDIO_BYTES_PER_SAMPLE 2

// frames per second
#define OOWIN32_AUDIO_BUFFER_FPS 44100
 
// 2 seconds
#define OOWIN32_AUDIO_BUFFER_SIZE 176400
//#define OOWIN32_FRAMES_OF_LATENCY 1470
#define OOWIN32_FRAMES_OF_LATENCY 735

typedef struct {
	LPDIRECTSOUNDBUFFER audioBuffer;
	ooint nextWritePosition;

} win32audiodata;

static win32audiodata audioData;

oouint win32AudioBufferSize() {
	return OOWIN32_AUDIO_BUFFER_SIZE;
}

oouint win32AudioFramesPerSecond() {
	return OOWIN32_AUDIO_BUFFER_FPS;
}

void win32ClearBuffer() {
    VOID * region1;
    DWORD region1Size;
    VOID * region2;
    DWORD region2Size;
    if( SUCCEEDED(audioData.audioBuffer->lpVtbl->Lock(audioData.audioBuffer, 0, OOWIN32_AUDIO_BUFFER_SIZE, &region1, &region1Size, &region2, &region2Size, 0)) ) {
		if( region1 ) {
			oobyte* destSample = (oobyte*) region1;
			for(DWORD i = 0; i < region1Size; i++) {
				*destSample++ = 0;
			}
		}
		
        if( region2 ) {
			oobyte* destSample = (oobyte*) region2;
			for(DWORD i = 0; i < region2Size; i++) {
				*destSample++ = 0;
			}
		}

        audioData.audioBuffer->lpVtbl->Unlock(audioData.audioBuffer, region1, region1Size, region2, region2Size);
    }
}


void win32AudioOutput(ooshort * data, oouint numOfFrames) {
	// TODO
	DWORD uPlayCursor = 0;
	DWORD uWriteCursor = 0;
	oouint latencyBytes = OOWIN32_FRAMES_OF_LATENCY*2*2; // two channels, two bytes per sample
	if(audioData.nextWritePosition<0) {
		audioData.nextWritePosition = latencyBytes;
	}
	oouint bytesToWrite = numOfFrames*2*2;
	oobyte* byteData = (oobyte*) data;

	ooint playCursor, writeCursor;

	if( SUCCEEDED(audioData.audioBuffer->lpVtbl->GetCurrentPosition(audioData.audioBuffer, &uPlayCursor, &uWriteCursor)) ) {
		audioData.nextWritePosition = audioData.nextWritePosition%OOWIN32_AUDIO_BUFFER_SIZE;

		playCursor = (ooint) uPlayCursor;
		writeCursor = (ooint) uWriteCursor;
		if( playCursor>writeCursor ) {
			// the write cursor wrapped around
			playCursor -= OOWIN32_AUDIO_BUFFER_SIZE;
		}


		if( audioData.nextWritePosition>=playCursor && audioData.nextWritePosition<writeCursor ) {
			// we can't write in here!
			audioData.nextWritePosition = writeCursor+latencyBytes;
		}

		// perform the actutal write
		VOID * region1;
		DWORD region1Size;
		VOID * region2;
		DWORD region2Size;
		//DWORD dataIndex = 0;
		if(SUCCEEDED(audioData.audioBuffer->lpVtbl->Lock(audioData.audioBuffer,audioData.nextWritePosition,bytesToWrite,&region1,&region1Size,&region2,&region2Size,0))) {
			if(region1) {
				oobyte* destSample = (oobyte*)region1;
				for(DWORD i = 0; i < region1Size; i++) {
					*destSample++ = *byteData++;
					//*destSample++ = byteData[dataIndex++];
				}
			}

			if(region2) {
				oobyte* destSample = (oobyte*)region2;
				for(DWORD i = 0; i < region2Size; i++) {
					*destSample++ = *byteData++;
					//*destSample++ = byteData[dataIndex++];
				}
			}

			audioData.audioBuffer->lpVtbl->Unlock(audioData.audioBuffer,region1,region1Size,region2,region2Size);
		}


		audioData.nextWritePosition += bytesToWrite;
	}
}

void win32AudioInit(HWND window) {
	
    HMODULE dSoundLibrary = LoadLibraryA("dsound.dll");
      
	directSoundCreate_function* directSoundCreate = (directSoundCreate_function *) GetProcAddress(dSoundLibrary, "DirectSoundCreate");

	
	LPDIRECTSOUND directSound;
	
	if( directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0)) ) {
		WAVEFORMATEX waveFormat;
		ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));

		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = 2;
		waveFormat.nSamplesPerSec = OOWIN32_AUDIO_BUFFER_FPS;
		waveFormat.wBitsPerSample = (WORD) (OOWIN32_AUDIO_BYTES_PER_SAMPLE*8);
		waveFormat.nBlockAlign = (waveFormat.nChannels*waveFormat.wBitsPerSample) / 8;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec*waveFormat.nBlockAlign;
		waveFormat.cbSize = 0;

		if( SUCCEEDED(directSound->lpVtbl->SetCooperativeLevel(directSound, window, DSSCL_PRIORITY)) ) {
			DSBUFFERDESC bufferDescription;
			ZeroMemory(&bufferDescription, sizeof(DSBUFFERDESC));
			bufferDescription.dwSize = sizeof(bufferDescription);
			bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

			LPDIRECTSOUNDBUFFER primaryBuffer = NULL;
			if( SUCCEEDED(directSound->lpVtbl->CreateSoundBuffer(directSound, &bufferDescription, &primaryBuffer, 0))) {
				HRESULT error =  primaryBuffer->lpVtbl->SetFormat(primaryBuffer, &waveFormat);
				if( error!=S_OK ) {
					return;
				}
			}
		}

		DSBUFFERDESC bufferDescription;
		ZeroMemory(&bufferDescription, sizeof(DSBUFFERDESC));

		bufferDescription.dwSize = sizeof(bufferDescription);
		bufferDescription.dwFlags = 0;
		bufferDescription.dwBufferBytes = OOWIN32_AUDIO_BUFFER_SIZE;
		bufferDescription.lpwfxFormat = &waveFormat;
		HRESULT error =  directSound->lpVtbl->CreateSoundBuffer(directSound, &bufferDescription, &audioData.audioBuffer, 0);
		if( error!=S_OK ) {
			audioData.audioBuffer = NULL;
		}
	}

	win32ClearBuffer();

	audioData.nextWritePosition = -1;
	
	audioData.audioBuffer->lpVtbl->Play(audioData.audioBuffer, 0, 0, DSBPLAY_LOOPING);
}

void win32AudioDestroy() {
	if( audioData.audioBuffer ) {
		audioData.audioBuffer->lpVtbl->Release(audioData.audioBuffer);
		audioData.audioBuffer = NULL;
	}
}