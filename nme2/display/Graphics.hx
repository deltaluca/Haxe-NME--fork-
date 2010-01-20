package nme2.display;

class Graphics
{
   var nmeHandle:Dynamic;

   public function new(inHandle:Dynamic)
	{
	   nmeHandle = inHandle;
	}

	public function beginFill(color:Int, alpha:Float = 1.0)
	{
		nme_gfx_begin_fill(nmeHandle,color,alpha);
	}

   public function lineStyle(?thickness:Null<Float>, color:Int = 0, alpha:Float = 1.0,
	                   pixelHinting:Bool = false, ?scaleMode:LineScaleMode, ?caps:CapsStyle,
							 ?joints:JointStyle, miterLimit:Float = 3) : Void
	{
		nme_gfx_line_style(nmeHandle, thickness, color, alpha, pixelHinting,
			scaleMode==null ?  0 : Type.enumIndex(scaleMode),
			caps==null ?  0 : Type.enumIndex(caps),
			joints==null ?  0 : Type.enumIndex(joints),
			miterLimit );
	}



	public function moveTo(inX:Float, inY:Float)
	{
		nme_gfx_move_to(nmeHandle,inX,inY);
	}

	public function lineTo(inX:Float, inY:Float)
	{
		nme_gfx_line_to(nmeHandle,inX,inY);
	}

	public function curveTo(inCX:Float, inCY:Float, inX:Float, inY:Float)
	{
		nme_gfx_curve_to(nmeHandle,inCX,inCY,inX,inY);
	}

	public function arcTo(inCX:Float, inCY:Float, inX:Float, inY:Float)
	{
		nme_gfx_arc_to(nmeHandle,inCX,inCY,inX,inY);
	}

	public function drawEllipse(inX:Float, inY:Float, inWidth:Float, inHeight:Float)
	{
		nme_gfx_draw_ellipse(nmeHandle,inX,inY,inWidth,inHeight);
	}


   static var nme_gfx_begin_fill = nme2.Loader.load("nme_gfx_begin_fill",3);
   static var nme_gfx_line_style = nme2.Loader.load("nme_gfx_line_style",-1);

   static var nme_gfx_move_to = nme2.Loader.load("nme_gfx_move_to",3);
   static var nme_gfx_line_to = nme2.Loader.load("nme_gfx_line_to",3);
   static var nme_gfx_curve_to = nme2.Loader.load("nme_gfx_curve_to",5);
   static var nme_gfx_arc_to = nme2.Loader.load("nme_gfx_arc_to",5);
   static var nme_gfx_draw_ellipse = nme2.Loader.load("nme_gfx_draw_ellipse",5);
}
