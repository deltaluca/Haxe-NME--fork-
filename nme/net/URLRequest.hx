package nme.net;


#if flash
@:native ("flash.net.URLRequest")
@:final extern class URLRequest {
	var contentType : String;
	var data : Dynamic;
	var digest : String;
	var method : String;
	var requestHeaders : Array<URLRequestHeader>;
	var url : String;
	function new(?url : String) : Void;
}
#else



class URLRequest
{
   public static inline var AUTH_BASIC         = 0x0001;
   public static inline var AUTH_DIGEST        = 0x0002;
   public static inline var AUTH_GSSNEGOTIATE  = 0x0004;
   public static inline var AUTH_NTLM          = 0x0008;
   public static inline var AUTH_DIGEST_IE     = 0x0010;
   public static inline var AUTH_DIGEST_ANY    = 0x000f;


   public var url(default,null):String;
   public var verbose:Bool;
   public var userPassword:String;
   public var authType:Int;
   public var cookieString:String;

   public function new(?inURL:String)
   {
      if (inURL!=null)
         url = inURL;
      verbose = false;
      userPassword="";
      cookieString="";
      authType = 0;
   }

   public function basicAuth(inUser:String, inPasswd:String)
   {
      authType = AUTH_BASIC;
      userPassword = inUser + ":" + inPasswd;
   }

   public function digestAuth(inUser:String, inPasswd:String)
   {
      authType = AUTH_DIGEST;
      userPassword = inUser + ":" + inPasswd;
   }
}
#end