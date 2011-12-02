package nme.filters;
#if (cpp||neko)

class ColorMatrixFilter extends BitmapFilter
{
	private var quality:Int;
	private var cmx:Array<Float>;

	public function new(cmx:Array<Float>=null, inQuality:Int=1)
	{
		super("ColorMatrixFilter");
		if(cmx==null) cmx = [1.0,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,1,0];
		this.cmx = cmx;
		quality = inQuality;
	}

	override public function clone():BitmapFilter
	{
		return new ColorMatrixFilter(cmx,quality);
	}
}

#else
//typedef flash
#end
