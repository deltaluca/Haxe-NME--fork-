package nme.display;
#if (cpp || neko)


import nme.events.Event;
import nme.geom.Point;
import nme.errors.RangeError;
import nme.Loader;


class DisplayObjectContainer extends InteractiveObject
{	
	
	public var mouseChildren(nmeGetMouseChildren, nmeSetMouseChildren):Bool;
	public var numChildren(nmeGetNumChildren, null):Int;
	public var tabChildren(nmeGetTabChildren, nmeSetTabChildren):Bool;
	//public var textSnapshot(nmeGetTextSnapshot, null):TextSnapshot; // not implemented
	
	private var nmeChildren:Array<DisplayObject>;
	
	
	public function new(inHandle:Dynamic, inType:String)
	{	
		super(inHandle, inType);
		nmeChildren = [];
	}
	
	
	public function addChild(child:DisplayObject):DisplayObject
	{	
		if (child == this)
		{	
			throw "Adding to self";	
		}
		
		if (child.nmeParent == this)
		{	
			setChildIndex(child, nmeChildren.length - 1);
		}
		else
		{	
			child.nmeSetParent(this);
			nmeChildren.push(child);
			nme_doc_add_child(nmeHandle, child.nmeHandle);
		}
		
		return child;
	}
	
	
	public function addChildAt(child:DisplayObject, index:Int):DisplayObject
	{	
		addChild(child);
		setChildIndex(child, index);
		
		return child;
	}
	
	
	public function areInaccessibleObjectsUnderPoint(point:Point):Bool
	{		
		return false;
	}
	
	
	public function contains(child:DisplayObject):Bool
	{	
		if (child == null)
			return false;
		
		if (this == child)
			return true;
		
		for (c in nmeChildren)
			if (c == child)
				return true;
		
		return false;	
	}
	
	
	public function getChildAt(index:Int):DisplayObject
	{	
		if (index >= 0 && index < nmeChildren.length)
			return nmeChildren[index];
		
		// TODO
		throw new RangeError("getChildAt : index out of bounds " + index + "/" + nmeChildren.length);
		
		return null;
	}

	
	public function getChildByName(name:String):DisplayObject
	{	
		for (c in nmeChildren)
			if (name == c.name)
				return c;
		return null;	
	}
	
	
	public function getChildIndex(child:DisplayObject):Int
	{	
		for (i in 0...nmeChildren.length)
			if (nmeChildren[i] == child)
				return i;	
		return -1;
	}

	
	public function getObjectsUnderPoint(point:Point):Array<DisplayObject>
	{	
		var result = new Array<DisplayObject>();
		nmeGetObjectsUnderPoint(point, result);
		return result;
	}
	

	override public function nmeBroadcast(inEvt:Event)
	{	
		var i = 0;
		
		if (nmeChildren.length > 0)
			while (true)
			{	
				var child = nmeChildren[i];
				child.nmeBroadcast (inEvt);
				
				if (i >= nmeChildren.length)
					break;
				
				if (nmeChildren[i] == child)
				{	
					i++;
					if (i >= nmeChildren.length)
						break;	
				}
			}
		
		dispatchEvent(inEvt);
	}
	
	
	override function nmeFindByID(inID:Int):DisplayObject
	{	
		if (nmeID == inID)
			return this;
		
		for (child in nmeChildren)
		{	
			var found = child.nmeFindByID(inID);
			
			if (found != null)
				return found;
		}
		
		return super.nmeFindByID(inID);
	}
	
	
	public override function nmeGetObjectsUnderPoint(point:Point, result:Array<DisplayObject>)
	{	
		super.nmeGetObjectsUnderPoint(point, result);
		
		for (child in nmeChildren)
			nmeGetObjectsUnderPoint(point, result);
	}
	
	
	override function nmeOnAdded(inObj:DisplayObject, inIsOnStage:Bool)
	{	
		super.nmeOnAdded(inObj, inIsOnStage);
		
		for (child in nmeChildren)
			child.nmeOnAdded(inObj, inIsOnStage);
	}
	
	
	override function nmeOnRemoved(inObj:DisplayObject, inWasOnStage:Bool)
	{	
		super.nmeOnRemoved(inObj, inWasOnStage);
		
		for (child in nmeChildren)
			child.nmeOnRemoved(inObj, inWasOnStage);
	}
	
	
	/**
	 * @private
	 */
	public function nmeRemoveChildFromArray(child:DisplayObject)
	{
		var i = getChildIndex(child);
		
		if (i >= 0)
		{	
			nme_doc_remove_child(nmeHandle, i);
			nmeChildren.splice(i, 1);	
		}
	}

	
	public function removeChild(child:DisplayObject):DisplayObject
	{	
		var c = getChildIndex(child);
		
		if (c >= 0)
		{	
			child.nmeSetParent(null);
			return child;
		}
		
		return null;
	}
	
	
	public function removeChildAt(index:Int):DisplayObject
	{	
		if (index >= 0 && index < nmeChildren.length)
		{	
			var result = nmeChildren[index];
			result.nmeSetParent(null);
			return result;
		}
		
		return null;
	}
	
	
	public function setChildIndex(child:DisplayObject, index:Int):Void
	{	
		if (index > nmeChildren.length)
			throw "Invalid index position " + index;
		
		var s:DisplayObject = null;
		var orig = getChildIndex(child);
		
		if (orig < 0)
		{	
			var msg = "setChildIndex : object " + child.toString() + " not found.";
			
			if (child.nmeParent == this)
			{	
				var realindex = -1;
				
				for (i in 0...nmeChildren.length)
				{	
					if (nmeChildren[i] == child)
					{	
						realindex = i;
						break;	
					}
				}
				
				if (realindex != -1)
					msg += "Internal error: Real child index was " + Std.string(realindex);
				else
					msg += "Internal error: Child was not in nmeChildren array!";	
			}
			
			throw msg;
		}
		
		nme_doc_set_child_index(nmeHandle, child.nmeHandle, index);
		
		if (index < orig) // move down ...
		{ 
			var i = orig;
			
			while (i > index)
			{	
				nmeChildren[i] = nmeChildren[i - 1];
				i--;	
			}
			
			nmeChildren[index] = child;
			
		}
		else if (orig < index) // move up ...
		{ 
			var i = orig;
			while (i < index)
			{	
				nmeChildren[i] = nmeChildren[i + 1];
				i++;	
			}
			
			nmeChildren[index] = child;
		}
	}
	
	
	public function swapChildren(child1:DisplayObject, child2:DisplayObject):Void
	{	
		var idx1 = getChildIndex(child1);
		var idx2 = getChildIndex(child2);
		if (idx1 < 0 || idx2 < 0)
			throw "swapChildren:Could not find children";
		swapChildrenAt(idx1, idx2);
	}
	
	
	public function swapChildrenAt(index1:Int, index2:Int):Void
	{	
		if (index1 < 0 || index2 < 0 || index1 > nmeChildren.length || index2 > nmeChildren.length)
			throw new RangeError ("swapChildrenAt : index out of bounds");
		
		if (index1 == index2)
			return;
		
		var tmp = nmeChildren[index1];
		nmeChildren[index1] = nmeChildren[index2];
		nmeChildren[index2] = tmp;
		nme_doc_swap_children(nmeHandle, index1, index2);
	}
	
	
	
	// Getters & Setters
	
	
	
	private function nmeGetMouseChildren():Bool { return nme_doc_get_mouse_children(nmeHandle); }
	private function nmeSetMouseChildren(inVal:Bool):Bool
	{
		nme_doc_set_mouse_children(nmeHandle, inVal);
		return inVal;
	}
	
	
	private function nmeGetNumChildren():Int { return nmeChildren.length; }
	private function nmeGetTabChildren() { return false; }
	private function nmeSetTabChildren(inValue:Bool) { return false; }
	
	
	
	// Native Methods
	
	
	
	private static var nme_create_display_object_container = Loader.load("nme_create_display_object_container", 0);
	private static var nme_doc_add_child = Loader.load("nme_doc_add_child", 2);
	private static var nme_doc_remove_child = Loader.load("nme_doc_remove_child", 2);
	private static var nme_doc_set_child_index = Loader.load("nme_doc_set_child_index", 3);
	private static var nme_doc_get_mouse_children = Loader.load("nme_doc_get_mouse_children", 1);
	private static var nme_doc_set_mouse_children = Loader.load("nme_doc_set_mouse_children", 2);
	private static var nme_doc_swap_children = Loader.load("nme_doc_swap_children", 3);
	
}


#else
typedef DisplayObjectContainer = flash.display.DisplayObjectContainer;
#end