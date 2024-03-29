package nme.system;
#if (cpp || neko)


import nme.Loader;


/**
 * ...
 * @author Joshua Granick
 */
class Capabilities
{
	
	public static var pixelAspectRatio(nmeGetPixelAspectRatio, null):Float;
	public static var screenDPI(nmeGetScreenDPI, null):Float;
	public static var screenResolutionX(nmeGetScreenResolutionX, null):Float;
	public static var screenResolutionY(nmeGetScreenResolutionY, null):Float;
	public static var screenResolutions(nmeGetScreenResolutions, null):Array<Array<Int>>;
	
	
	
	// Getters & Setters
	
	
	
	private static function nmeGetPixelAspectRatio():Float { return nme_capabilities_get_pixel_aspect_ratio(); }
	private static function nmeGetScreenDPI():Float { return nme_capabilities_get_screen_dpi(); }
	private static function nmeGetScreenResolutionX():Float { return nme_capabilities_get_screen_resolution_x(); }
	private static function nmeGetScreenResolutionY():Float { return nme_capabilities_get_screen_resolution_y(); }
	
	
	private static function nmeGetScreenResolutions():Array<Array<Int>>
	{
		var res:Array<Int> = nme_capabilities_get_screen_resolutions();
		
		if (res == null) 
			return new Array<Array<Int>>();
		
		var out:Array<Array<Int>> = new Array<Array<Int>>();
		
		for (c in 0...Std.int(res.length / 2))
		{
			out.push( [ res[ c * 2 ], res[ c * 2 + 1 ] ] );
		}
		
		return out;
	}
	
	
	
	// Native Methods
	
	
	
	private static var nme_capabilities_get_pixel_aspect_ratio = Loader.load("nme_capabilities_get_pixel_aspect_ratio", 0);
	private static var nme_capabilities_get_screen_dpi = Loader.load("nme_capabilities_get_screen_dpi", 0);
	private static var nme_capabilities_get_screen_resolution_x = Loader.load("nme_capabilities_get_screen_resolution_x", 0);
	private static var nme_capabilities_get_screen_resolution_y = Loader.load("nme_capabilities_get_screen_resolution_y", 0);
	private static var nme_capabilities_get_screen_resolutions = Loader.load("nme_capabilities_get_screen_resolutions", 0 );
	
}


#elseif !jeash
typedef Capabilities = flash.system.Capabilities;
#else


class Capabilities
{
	
	public static var pixelAspectRatio(jeashGetPixelAspectRatio, null):Float;
	public static var screenDPI(jeashGetScreenDPI, null):Float;
	public static var screenResolutionX(jeashGetScreenResolutionX, null):Float;
	public static var screenResolutionY(jeashGetScreenResolutionY, null):Float;
	
	
	
	// Getters & Setters
	
	
	
	private static function jeashGetPixelAspectRatio():Float { return 1; }
	private static function jeashGetScreenDPI():Float { return 72; }
	private static function jeashGetScreenResolutionX():Float { return 0; }
	private static function jeashGetScreenResolutionY():Float { return 0; }
	
}
#end