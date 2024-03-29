package nme.display;
#if (cpp || neko)


import nme.net.URLRequest;
import nme.display.DisplayObject;
import nme.display.Bitmap;
import nme.display.BitmapData;
import nme.display.LoaderInfo;
import nme.display.Shape;
import nme.display.Sprite;
import nme.events.Event;
import nme.events.IOErrorEvent;
import nme.utils.ByteArray;


/**
* @author   Hugh Sanderson
* @author   Niel Drummond
* @author   Russell Weir
* @author   Joshua Harlan Lifton
* @todo init, open, progress events
* @todo Complete LoaderInfo initialization
* @todo Cancel previous load request if new load request is made before completion.
**/
class Loader extends Sprite
{	
	
	public var content(default, null):DisplayObject;
	public var contentLoaderInfo(default, null):LoaderInfo;
	
	private var nmeImage:BitmapData;
	private var nmeSWF:MovieClip;
	

	public function new()
	{	
		super();
		contentLoaderInfo = LoaderInfo.create(this);
		// Make sure we get in first...
		contentLoaderInfo.addEventListener(Event.COMPLETE, onData);
	}
	
	
	private function doLoad(inBytes:ByteArray)
	{	
		try
		{	
			nmeImage = BitmapData.loadFromBytes(inBytes);
			
			var bmp = new Bitmap(nmeImage);
			content = bmp;
			contentLoaderInfo.content = bmp;
			
			while (numChildren > 0)
			{	
				removeChildAt(0);
			}
			
			addChild(bmp);
			contentLoaderInfo.dispatchEvent(new Event(Event.COMPLETE));
		}
		catch (e:Dynamic)
		{	
			//trace("Error " + e);
			contentLoaderInfo.DispatchIOErrorEvent();
			return;	
		}
	}

	
	public function load(request:URLRequest)
	{	
		// No "loader context" in nme
		contentLoaderInfo.load(request);	
	}
	
	
	public function loadBytes(bytes:ByteArray)
	{	
		// No "loader context" in nme
      doLoad(bytes);
	}
	
	
	public function unload()
	{	
		if (numChildren > 0)
		{	
			while (numChildren > 0)
			{	
				removeChildAt(0);
			}
			
			untyped
			{	
				contentLoaderInfo.url = null;
				contentLoaderInfo.contentType = null;
				contentLoaderInfo.content = null;
				contentLoaderInfo.bytesLoaded = contentLoaderInfo.bytesTotal = 0;
				contentLoaderInfo.width = 0;
				contentLoaderInfo.height = 0;	
			}
			
			dispatchEvent(new Event(Event.UNLOAD));
		}
	}
	
	
	
	// Event Handlers
	
	
	
	private function onData(_)
	{
		doLoad(contentLoaderInfo.bytes);
	}
	
}


#else
typedef Loader = flash.display.Loader;
#end
