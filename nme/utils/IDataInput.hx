package nme.utils;
#if (cpp || neko)


interface IDataInput
{
	
	public var bytesAvailable(nmeGetBytesAvailable, null):Int;
	public var endian(nmeGetEndian, nmeSetEndian):String;
	
	public function readBoolean():Bool;
	public function readByte():Int;
	public function readBytes(outData:ByteArray, inOffset:Int = 0, inLen:Int = 0):Void;
	public function readDouble():Float;
	public function readFloat():Float;
	public function readInt():Int;
	
	// not implemented ...
	//var objectEncoding : UInt;
	//public function readMultiByte(length : Int, charSet:String):String;
	//public function readObject():Dynamic;
	
	public function readShort():Int;
	public function readUnsignedByte():Int;
	public function readUnsignedInt():Int;
	public function readUnsignedShort():Int;
	public function readUTF():String;
	public function readUTFBytes(inLen:Int):String;
	
	public function nmeGetBytesAvailable():Int;
	public function nmeGetEndian():String;
	public function nmeSetEndian(s:String):String;
	
}


#else
typedef IDataInput = flash.utils.IDataInput;
#end