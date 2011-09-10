#if flash


package nme.display;


/*
@:native ("flash.display.StageDisplayState")
@:fakeEnum(String) extern enum StageDisplayState {
	FULL_SCREEN;
	FULL_SCREEN_INTERACTIVE;
	NORMAL;
}
*/

class StageDisplayState {
	
	public static var FULL_SCREEN:String = "fullScreen";
	public static var FULL_SCREEN_INTERACTIVE:String = "fullScreenInteractive";
	public static var NORMAL:String = "normal";
	
}


#else


package nme.display;

enum StageDisplayState
{
   NORMAL;
   FULL_SCREEN;
   FULL_SCREEN_INTERACTIVE;
}


#end