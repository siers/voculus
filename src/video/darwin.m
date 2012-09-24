#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

#import "log.h"
#import "video.h"

extern struct video video;

@interface Capturer : NSThread {
  QTCaptureSession                  *mCaptureSession;
  QTCaptureDeviceInput              *mCaptureVideoDeviceInput;
  QTCaptureDecompressedVideoOutput  *mCaptureDecompressedVideoOutput;
}

- (int) main;
- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection;
- (void) captureDelete;
- (void) startCapture;
- (void) stopCapture;
@end


@implementation Capturer : NSThread

- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
  int i, j, w, h;
  unsigned char *argb;

  CVPixelBufferLockBaseAddress(videoFrame, 0);
  argb = (unsigned char *) CVPixelBufferGetBaseAddress(videoFrame);
  w    = CVPixelBufferGetWidth(videoFrame);
  h    = CVPixelBufferGetHeight(videoFrame);

  // your code here… with raw ARGB buffer
  CVPixelBufferUnlockBaseAddress(videoFrame, 0);
}

/* We'll create a capture session with the following device inputs & outputs - video device input, then start capturing. */
- (int) main
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  BOOL success = NO;
  NSError *error;

  // Find a video device
  QTCaptureDevice *videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
  success = [videoDevice open:&error];

  if (!success && videoDevice) {
    log("video device open unsuccessful, error: %s", [error localizedDescription]);
    exit(1);
  }
  log("Capturing from: %@", videoDevice);

  mCaptureSession = [[QTCaptureSession alloc] init];
  mCaptureVideoDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:videoDevice];

  success = [mCaptureSession addInput:mCaptureVideoDeviceInput error:&error];
  if (success != YES) {
    log("Input adding to session failed, error: %s", [error localizedDescription]);
    exit(1);
  }

  mCaptureDecompressedVideoOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
  [mCaptureDecompressedVideoOutput setPixelBufferAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                 [NSNumber numberWithDouble:video.width], (id)kCVPixelBufferWidthKey,
                                 [NSNumber numberWithDouble:video.height], (id)kCVPixelBufferHeightKey,
                            [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
                            nil]];
  [mCaptureDecompressedVideoOutput setDelegate:self];

  if ([mCaptureSession addOutput:mCaptureDecompressedVideoOutput error:&error] != YES) {
    log("Adding decomp. output to session failed, error: %s", [error localizedDescription]);
    exit(1);
  }

  [self startCapture];
  [pool release];

  return 0;
}

/* start capturing */
- (void) startCapture
{
  if ([[mCaptureVideoDeviceInput device] isOpen] == NO)
    return;

  [mCaptureSession startRunning];
}

/* stop capturing, and remove all device inputs/outputs */
- (void) captureDelete
{
  /* remove video device inputs */
  if ([[mCaptureVideoDeviceInput device] isOpen])
  {
    [[mCaptureVideoDeviceInput device] close];
    [mCaptureSession removeInput:mCaptureVideoDeviceInput];
    [mCaptureVideoDeviceInput release];
    mCaptureVideoDeviceInput = nil;
  }
}

/* stop capturing */
- (void) stopCapture
{
  if ([mCaptureSession isRunning] == YES)
  {
    [mCaptureSession stopRunning];
  }
}

@end

void
video_specific_init()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];

  [[[Capturer alloc] init] start];

  [NSApp run]; // Unlikely to return.
  [pool release];
}

void
video_specific_die()
{
  // should never happen.
}
