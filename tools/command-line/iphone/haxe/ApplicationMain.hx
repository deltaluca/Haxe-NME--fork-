import nme.Assets;

class ApplicationMain
{
   public static function main()
   {
      nme.Lib.setAssetBase("assets/");
      ::if (IPHONE_ORIENTATION!=null)::
      nme.display.Stage.setFixedOrientation(nme.display.Stage.Orientation::IPHONE_ORIENTATION::);
      ::end::
      nme.Lib.create(
           function(){ ::APP_MAIN::.main(); },
           ::WIN_WIDTH::, ::WIN_HEIGHT::,
           ::WIN_FPS::,
           ::WIN_BACKGROUND::,
             ( ::WIN_HARDWARE::   ? nme.Lib.HARDWARE  : 0) |
             ( ::WIN_RESIZEABLE:: ? nme.Lib.RESIZABLE : 0),
          "::APP_TITLE::",
		  "::APP_PACKAGE::"
          );
   }

   public static function getAsset(inName:String):Dynamic
   {
      ::foreach assets::
		if (inName=="::id::") {
			::if (type == "image")::
			return Assets.getBitmapData ("::id::");
			::elseif (type=="sound")::
			return Assets.getSound ("::id::");
			::elseif (type=="music")::
			return Assets.getSound ("::id::");
			::elseif (type== "font")::
			 return Assets.getFont ("::id::");
			::else::
			return Assets.getBytes ("::id::");
			::end::
		}
		::end::
		return null;
   }
}

