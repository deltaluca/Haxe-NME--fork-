#if flash


package nme.text;


@:native ("flash.text.TextSnapshot")
extern class TextSnapshot {
	var charCount(default,null) : Int;
	function new() : Void;
	function findText(beginIndex : Int, textToFind : String, caseSensitive : Bool) : Int;
	function getSelected(beginIndex : Int, endIndex : Int) : Bool;
	function getSelectedText(includeLineEndings : Bool = false) : String;
	function getText(beginIndex : Int, endIndex : Int, includeLineEndings : Bool = false) : String;
	function getTextRunInfo(beginIndex : Int, endIndex : Int) : Array<Dynamic>;
	function hitTestTextNearPos(x : Float, y : Float, maxDistance : Float = 0) : Float;
	function setSelectColor(hexColor : UInt = 16776960) : Void;
	function setSelected(beginIndex : Int, endIndex : Int, select : Bool) : Void;
}


#end