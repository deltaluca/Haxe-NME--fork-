package nme2.events;

import nme2.display.InteractiveObject;
import nme2.geom.Point;

class MouseEvent extends nme2.events.Event
{
   public var localX : Float;
   public var localY : Float;
   public var relatedObject : InteractiveObject;
   public var ctrlKey : Bool;
   public var altKey : Bool;
   public var shiftKey : Bool;
   public var buttonDown : Bool;
   public var delta : Int;
   public var commandKey : Bool;
   public var clickCount : Int;

   public var stageX : Float;
   public var stageY : Float;

   public function new(type : String,
            bubbles : Bool = true, 
            cancelable : Bool = false,
            in_localX : Float = 0,
            in_localY : Float = 0,
            in_relatedObject : InteractiveObject = null,
            in_ctrlKey : Bool = false,
            in_altKey : Bool = false,
            in_shiftKey : Bool = false,
            in_buttonDown : Bool = false,
            in_delta : Int = 0,
            in_commandKey:Bool = false,
            in_clickCount:Int = 0 )
   {
      super(type,bubbles,cancelable);

      localX = in_localX;
      localY = in_localY;
      relatedObject = in_relatedObject;
      ctrlKey = in_ctrlKey;
      altKey = in_altKey;
      shiftKey = in_shiftKey;
      buttonDown = in_buttonDown;
      delta = in_delta;
      commandKey = in_commandKey;
      clickCount = in_clickCount;
   }

   static var efLeftDown  =  0x0001;
   static var efShiftDown =  0x0002;
   static var efCtrlDown  =  0x0004;
   static var efAltDown   =  0x0008;
   static var efCommandDown = 0x0010;

   public static function nmeCreate(inType:String, inEvent:Dynamic,inLocal:Point)
   {
      var flags : Int = inEvent.flags;
      var evt = new MouseEvent(inType, true, false, inLocal.x, inLocal.y, null,
           (flags & efCtrlDown) != 0,
           (flags & efAltDown) != 0,
           (flags & efShiftDown) != 0,
           (flags & efLeftDown) != 0,
           0,0 );
      evt.stageX = inEvent.x;
      evt.stageY = inEvent.y;
      return evt;
   }

   public function updateAfterEvent()
   {
   }

   public static var CLICK : String = "click";
   public static var DOUBLE_CLICK : String = "doubleClick";
   public static var MOUSE_DOWN : String = "mouseDown";
   public static var MOUSE_MOVE : String = "mouseMove";
   public static var MOUSE_OUT : String = "mouseOut";
   public static var MOUSE_OVER : String = "mouseOver";
   public static var MOUSE_UP : String = "mouseUp";
   public static var MOUSE_WHEEL : String = "mouseWheel";
   public static var ROLL_OUT : String = "rollOut";
   public static var ROLL_OVER : String = "rollOver";
}

