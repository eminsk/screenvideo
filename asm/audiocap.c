#define COBJMACROS
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct {
    char riff[4];
    DWORD riffSize;
    char wave[4];
    char fmt[4];
    DWORD fmtSize;
    WORD formatTag;
    WORD channels;
    DWORD sampleRate;
    DWORD bytesPerSec;
    WORD blockAlign;
    WORD bitsPerSample;
    char data[4];
    DWORD dataSize;
} WavHeader;
#pragma pack(pop)

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    const char* outPath = argv[1];

    HANDLE hStopEvent = CreateEventA(NULL, TRUE, FALSE, "ScreenVideoAudioStopEvent");
    if (!hStopEvent) return 2;

    CoInitialize(NULL);

    IMMDeviceEnumerator* pEnum = NULL;
    if (FAILED(CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**)&pEnum)))
        return 3;

    IMMDevice* pDev = NULL;
    if (FAILED(IMMDeviceEnumerator_GetDefaultAudioEndpoint(pEnum, eRender, eConsole, &pDev)))
        return 4;

    IAudioClient* pClient = NULL;
    if (FAILED(IMMDevice_Activate(pDev, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pClient)))
        return 5;

    WAVEFORMATEX* pFormat = NULL;
    if (FAILED(IAudioClient_GetMixFormat(pClient, &pFormat)))
        return 6;

    if (FAILED(IAudioClient_Initialize(pClient, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, pFormat, NULL)))
        return 7;

    IAudioCaptureClient* pCapture = NULL;
    if (FAILED(IAudioClient_GetService(pClient, &IID_IAudioCaptureClient, (void**)&pCapture)))
        return 8;

    FILE* f = fopen(outPath, "wb");
    if (!f) return 9;

    WORD outChannels = pFormat->nChannels;
    DWORD outRate = pFormat->nSamplesPerSec;
    WavHeader hdr = {
        {'R','I','F','F'}, 0,
        {'W','A','V','E'},
        {'f','m','t',' '}, 16,
        1, outChannels, outRate,
        outRate * outChannels * 2,
        (WORD)(outChannels * 2), 16,
        {'d','a','t','a'}, 0
    };
    fwrite(&hdr, sizeof(hdr), 1, f);
    DWORD totalDataBytes = 0;

    IAudioClient_Start(pClient);

    DWORD startTime = GetTickCount();

    static short convBuf[8192];
    static BYTE silentBuf[8192] = {0};

    while (WaitForSingleObject(hStopEvent, 20) == WAIT_TIMEOUT) {
        UINT32 packetLen = 0;
        HRESULT hr = IAudioCaptureClient_GetNextPacketSize(pCapture, &packetLen);
        while (SUCCEEDED(hr) && packetLen > 0) {
            BYTE* pData = NULL;
            UINT32 numFrames = 0;
            DWORD flags = 0;
            hr = IAudioCaptureClient_GetBuffer(pCapture, &pData, &numFrames, &flags, NULL, NULL);
            if (SUCCEEDED(hr)) {
                if (numFrames > 0) {
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                        DWORD bytes = numFrames * outChannels * 2;
                        while (bytes > 0) {
                            DWORD toWrite = bytes > sizeof(silentBuf) ? sizeof(silentBuf) : bytes;
                            fwrite(silentBuf, 1, toWrite, f);
                            totalDataBytes += toWrite;
                            bytes -= toWrite;
                        }
                    } else if (pFormat->wBitsPerSample == 32) {
                        float* src = (float*)pData;
                        UINT32 totalSamples = numFrames * outChannels;
                        UINT32 done = 0;
                        while (done < totalSamples) {
                            UINT32 chunk = totalSamples - done;
                            if (chunk > sizeof(convBuf)/sizeof(convBuf[0]))
                                chunk = sizeof(convBuf)/sizeof(convBuf[0]);
                            for (UINT32 i = 0; i < chunk; i++) {
                                float val = src[done + i];
                                if (val > 1.0f) val = 1.0f;
                                else if (val < -1.0f) val = -1.0f;
                                convBuf[i] = (short)(val * 32767.0f);
                            }
                            DWORD bytes = chunk * sizeof(short);
                            fwrite(convBuf, 1, bytes, f);
                            totalDataBytes += bytes;
                            done += chunk;
                        }
                    } else {
                        DWORD bytes = numFrames * pFormat->nBlockAlign;
                        fwrite(pData, 1, bytes, f);
                        totalDataBytes += bytes;
                    }
                    IAudioCaptureClient_ReleaseBuffer(pCapture, numFrames);
                }
            } else {
                break;
            }
            hr = IAudioCaptureClient_GetNextPacketSize(pCapture, &packetLen);
        }

        DWORD elapsedMs = GetTickCount() - startTime;
        DWORD bytesPerSec = outRate * outChannels * 2;
        DWORD expectedBytes = (DWORD)(((ULONGLONG)elapsedMs * bytesPerSec) / 1000);
        if (expectedBytes > totalDataBytes) {
            DWORD diff = expectedBytes - totalDataBytes;
            while (diff > 0) {
                DWORD toWrite = diff > sizeof(silentBuf) ? sizeof(silentBuf) : diff;
                fwrite(silentBuf, 1, toWrite, f);
                totalDataBytes += toWrite;
                diff -= toWrite;
            }
        }
    }

    IAudioClient_Stop(pClient);

    hdr.dataSize = totalDataBytes;
    hdr.riffSize = sizeof(WavHeader) - 8 + totalDataBytes;
    fflush(f);
    fseek(f, 0, SEEK_SET);
    fwrite(&hdr, sizeof(hdr), 1, f);
    fflush(f);
    fclose(f);

    IAudioCaptureClient_Release(pCapture);
    CoTaskMemFree(pFormat);
    IAudioClient_Release(pClient);
    IMMDevice_Release(pDev);
    IMMDeviceEnumerator_Release(pEnum);
    CloseHandle(hStopEvent);
    CoUninitialize();
    return 0;
}
