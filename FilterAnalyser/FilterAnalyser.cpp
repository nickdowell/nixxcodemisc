
#include <Carbon/Carbon.h>
#include <Accelerate/Accelerate.h>
#include <vector>

#include "LowPassFilter.h"

//----------------------------------------------------------------------------------------

using std::vector;

typedef std::pair<float, float> graph_point;
vector<graph_point> graph_data;

const float  kSampleRate = 44100.f;
const size_t kNumSamples = 4096;
const float  kGraphScale = 24.f;

//----------------------------------------------------------------------------------------

OSStatus MyDrawEventHandler (EventHandlerCallRef myHandler, 
							 EventRef event, void *userData)
{
	assert( userData );
    CGContextRef	gc;
    HIRect			bounds;
	HIViewGetBounds((HIViewRef) userData, &bounds);
    GetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (CGContextRef), NULL, &gc);
	
	CGContextSetRGBFillColor( gc, 0, 0, 0, 1 );
	CGContextFillRect( gc, bounds );
	
	assert( graph_data.size() );
	float minX = log2f(graph_data.front().first);
	float maxX = log2f(graph_data.back().first);
	float scaleX = bounds.size.width / (maxX - minX);
	float scaleY = bounds.size.height / (kGraphScale * 2.f);
	
#define xPos( hz ) ((log2f(hz) - minX) * scaleX)
#define yPos( db ) ((kGraphScale - db) * scaleY)

#define DrawGridLineVertical( x ) \
	CGContextMoveToPoint(    gc, x, yPos(-kGraphScale) ); \
	CGContextAddLineToPoint( gc, x, yPos( kGraphScale) );

#define DrawGridLineHorizontal( y ) \
	CGContextMoveToPoint(    gc,                 0, y ); \
	CGContextAddLineToPoint( gc, bounds.size.width, y );

	// major grid lines
	CGContextBeginPath( gc );
	DrawGridLineHorizontal( yPos(0) );
	DrawGridLineVertical( xPos(10) );
	DrawGridLineVertical( xPos(100) );
	DrawGridLineVertical( xPos(1000) );
	DrawGridLineVertical( xPos(10000) );
	CGContextSetRGBStrokeColor( gc, 1, 1, 1, 1 );
	CGContextSetLineWidth( gc, 0.5 );
	CGContextStrokePath( gc );
	
	// minor grid lines
	CGContextBeginPath( gc );
	for (int i=-18; i<=18; i+=6) {
		DrawGridLineHorizontal( yPos(i) );
	}
	for (size_t i=2; i<10; i++) {
		DrawGridLineVertical( xPos(i) );
		DrawGridLineVertical( xPos(i * 10) );
		DrawGridLineVertical( xPos(i * 100) );
		DrawGridLineVertical( xPos(i * 1000) );
		DrawGridLineVertical( xPos(i * 10000) );
	}
	CGContextSetRGBStrokeColor( gc, 0.5, 0.5, 0.5, 1 );
	CGContextSetLineWidth( gc, 0.5 );
	CGContextStrokePath( gc );
	
	
	CGContextBeginPath( gc );
	CGContextMoveToPoint( gc, xPos(graph_data[0].first), yPos(graph_data[0].second) );
	for (size_t i=1; i<graph_data.size(); i++) {
		CGContextAddLineToPoint( gc, xPos(graph_data[i].first), yPos(graph_data[i].second) );
	}
	CGContextSetRGBStrokeColor( gc, 1, 1, 0, 1 );
	CGContextSetLineJoin( gc, kCGLineJoinMiter );
	CGContextSetLineWidth( gc, 2 );
	CGContextStrokePath( gc );

	return noErr;
}

//----------------------------------------------------------------------------------------

void runTest()
{
	vector<float> real( kNumSamples, 0.0f );
	vector<float> imag( kNumSamples, 0.0f );
	
	real[0] = 1.0f;
	LowPassFilter filter;
	filter.SetSampleRate( kSampleRate );
	filter.ProcessSamples( &real.front(), kNumSamples, 1000, 0.9 );

	////
	
	DSPSplitComplex cSplit;
	cSplit.realp = &real.front();
	cSplit.imagp = &imag.front();
	
	const vDSP_Length log2n = log2( kNumSamples );

	FFTSetup fftSetup =
	vDSP_create_fftsetup( log2n, FFT_RADIX2 );
	vDSP_fft_zip( fftSetup, &cSplit, 1, log2n, FFT_FORWARD );
	vDSP_destroy_fftsetup( fftSetup );
	
	for (size_t i=1; i<kNumSamples / 2; i++) {
		float magnitude = sqrtf( cSplit.realp[i] * cSplit.realp[i] + cSplit.imagp[i] * cSplit.imagp[i] );
		float db = 20.f * log10f( magnitude );
		float hz = i * kSampleRate / kNumSamples;
		graph_data.push_back( graph_point(hz, db) );
	}
}

//--------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	runTest();

    OSStatus	err;
	Rect		rect;
    WindowRef	window;
	ControlRef	userPane = 0;
	HIViewRef	contentView = NULL;
	
	const EventTypeSpec userPaneEvents[] = {
		{ kEventClassControl, kEventControlDraw }
	};
	
	SetRect(&rect, 0, 0, 1024, 512);
	err = CreateNewWindow(kDocumentWindowClass,
						  kWindowStandardDocumentAttributes
						  | kWindowStandardHandlerAttribute
						  | kWindowLiveResizeAttribute
						  | kWindowCompositingAttribute,
						  &rect, &window);

	// create user pane
	CreateUserPaneControl( window, &rect, 0, &userPane );
	InstallControlEventHandler( userPane, MyDrawEventHandler, GetEventTypeCount(userPaneEvents), userPaneEvents, userPane, NULL );
	HIViewFindByID( HIViewGetRoot(window), kHIViewWindowContentID, &contentView );
	HIViewAddSubview( contentView, userPane );
	
	// for automatic resizing of userPane
	HILayoutInfo layoutInfo = { 0 };
	layoutInfo.version = kHILayoutInfoVersionZero;
	layoutInfo.binding.top.kind = kHILayoutBindTop;
	layoutInfo.binding.left.kind = kHILayoutBindLeft;
	layoutInfo.binding.right.kind = kHILayoutBindRight;
	layoutInfo.binding.bottom.kind = kHILayoutBindBottom;
	HIViewSetLayoutInfo(userPane, &layoutInfo);	
    
    // Position new windows in a staggered arrangement on the main screen
    RepositionWindow( window, NULL, kWindowCascadeOnMainScreen );
    
    // The window was created hidden, so show it
    ShowWindow( window );
	
	RunApplicationEventLoop();
    
    return 0;
}
