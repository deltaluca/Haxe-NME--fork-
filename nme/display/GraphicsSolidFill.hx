package nme.display;
#if (cpp || neko)


import nme.Loader;


class GraphicsSolidFill extends IGraphicsData
{	
	
	public function new(color:Int = 0, alpha:Float = 1.0)
	{	
		super(nme_graphics_solid_fill_create(color, alpha));
	}
	
	
	private static var nme_graphics_solid_fill_create = Loader.load("nme_graphics_solid_fill_create", 2);
	
}


#else
//typedef GraphicsSolidFill = flash.display.GraphicsSolidFill;
#end