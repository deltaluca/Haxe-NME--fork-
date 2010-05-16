#if flash
import flash.Lib;
import flash.events.MouseEvent;
import flash.events.Event;
import flash.display.DisplayObject;
import flash.display.IGraphicsData;
import flash.display.BitmapData;
import flash.display.Sprite;
import flash.display.Shape;
import flash.display.Bitmap;
import flash.geom.Matrix;
import flash.geom.Rectangle;
import flash.utils.ByteArray;
#else
import nme.Lib;
import nme.events.MouseEvent;
import nme.events.Event;
import nme.display.DisplayObject;
import nme.display.Bitmap;
import nme.display.IGraphicsData;
import nme.display.BitmapData;
import nme.display.Sprite;
import nme.display.Shape;
import nme.geom.Matrix;
import nme.geom.Rectangle;
import nme.utils.ByteArray;
#end


class Sample extends Sprite
{

public function new(image1:BitmapData, image2:BitmapData, image3:BitmapData)
{
   super();
   Lib.current.addChild(this);

   addChild( new Bitmap(image1) );
   var shape = new Shape();
   addChild(shape);

	var copy = image1.clone();
	var bytes:ByteArray = copy.getPixels(new Rectangle(0,0,copy.width,copy.height));
	trace(bytes.length);


   var gfx = shape.graphics;
   gfx.lineStyle(1,0x000000);
   var mtx = new Matrix();
   gfx.beginBitmapFill(image1,mtx,true,true);
   gfx.drawRect(0,0,image1.width,image1.height);

   var mtx = new Matrix();
   mtx.translate(-200,-100);
   mtx.scale(5,5);
   gfx.beginBitmapFill(image1,mtx,false,false);
   gfx.drawRect(100,100,200,200);

   shape.x = 100;
   shape.y = 100;
   var mtx = new Matrix();
   mtx.translate(-50,-50);
   gfx.beginBitmapFill(image2,mtx,true,true);
   gfx.drawRect(-50,-50,image2.width,image2.height);
   var shape2 = new Shape();
   addChild(shape2);


   var gfx = shape2.graphics;
   var mtx = new Matrix();
   gfx.beginBitmapFill(image3,mtx,true,true);
   gfx.drawRect(0,0,image3.width,image3.height);
   shape2.x = 200;
   shape2.y = 200;


   stage.addEventListener( Event.ENTER_FRAME, function(_)
      {
         shape.rotation = shape.rotation + 0.01;
      } );

}


public static function main()
{
#if flash
   var image1:BitmapData;
   var image2:BitmapData;
   var image3:BitmapData;

   var loader = new flash.display.Loader();
   loader.contentLoaderInfo.addEventListener(flash.events.Event.COMPLETE,
      function(e:flash.events.Event)
      {
         image1 = untyped loader.content.bitmapData;
         var loader = new flash.display.Loader();
          loader.contentLoaderInfo.addEventListener(flash.events.Event.COMPLETE,
          function(e:flash.events.Event)
          {
             image2 = untyped loader.content.bitmapData;
             var loader = new flash.display.Loader();
             loader.contentLoaderInfo.addEventListener(flash.events.Event.COMPLETE,
              function( e:flash.events.Event)
              {
                 image3 = untyped loader.content.bitmapData;
                 new Sample(image1,image2,image3);
              });
              loader.load(new flash.net.URLRequest("Image2.png"));
          });
          loader.load(new flash.net.URLRequest("Image1.png"));

    });
    loader.load(new flash.net.URLRequest("Image.jpg"));


#else
   Lib.create(function() {
        var image1 = BitmapData.load("Image.jpg");
        var image2 = BitmapData.load("Image1.png");
        var image3 = BitmapData.load("Image2.png");
	     new Sample(image1,image2,image3);
		  }
	    ,320,480,60,0xccccff,(0*Lib.HARDWARE) | Lib.RESIZABLE);
#end
}

}
