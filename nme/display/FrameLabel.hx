#if flash


package nme.display;


@:native ("flash.display.FrameLabel")
@:final extern class FrameLabel {
	var frame(default,null) : Int;
	var name(default,null) : String;
	function new(name : String, frame : Int) : Void;
}


#end