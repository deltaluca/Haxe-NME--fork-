package nme.geom;
#if (cpp || neko)


class Vector3D
{
	
	inline public static var X_AXIS(nmeGetX_AXIS, null):Vector3D;
	inline public static var Y_AXIS(nmeGetY_AXIS, null):Vector3D;
	inline public static var Z_AXIS(nmeGetZ_AXIS, null):Vector3D;
	
	inline public var length(nmeGetLength, null):Float;
	inline public var lengthSquared(nmeGetLengthSquared, null):Float;
	public var w:Float;
	public var x:Float;
	public var y:Float;
	public var z:Float;
	
	
	public function new(?x:Float = 0., ?y:Float = 0., ?z:Float = 0., ?w:Float = 0.)
	{
		this.w = w;
		this.x = x;
		this.y = y;
		this.z = z;
	}
	
	
	inline public function add(a:Vector3D):Vector3D
	{
		return new Vector3D(this.x + a.x, this.y + a.y, this.z + a.z);
	}
	
	
	inline public static function angleBetween(a:Vector3D, b:Vector3D):Float
	{
		var a0 = a.clone();
		a0.normalize();
		var b0 = b.clone();
		b0.normalize();
		return Math.acos(a0.dotProduct(b0));
	}
	
	
	inline public function clone():Vector3D
	{
		return new Vector3D(x, y, z, w);
	}
	
	
	inline public function crossProduct(a:Vector3D):Vector3D
	{
		return new Vector3D(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x, 1);
	}
	
	
	inline public function decrementBy(a:Vector3D):Void
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
	}
	
	
	inline public static function distance(pt1:Vector3D, pt2:Vector3D):Float
	{
		var x:Float = pt2.x - pt1.x;
		var y:Float = pt2.y - pt1.y;
		var z:Float = pt2.z - pt1.z;
		
		return Math.sqrt(x * x + y * y + z * z);
	}
	
	
	inline public function dotProduct(a:Vector3D):Float
	{
		return x * a.x + y * a.y + z * a.z;
	}
	
	
	inline public function equals(toCompare:Vector3D, ?allFour:Bool = false):Bool
	{
		return x == toCompare.x && y == toCompare.y && z == toCompare.z && (!allFour || w == toCompare.w);
	}
	
	
	inline public function incrementBy(a:Vector3D):Void
	{
		x += a.x;
		y += a.y;
		z += a.z;
	}
	
	
	inline public function nearEquals(toCompare:Vector3D, tolerance:Float, ?allFour:Bool = false):Bool
	{
		return Math.abs(x - toCompare.x) < tolerance && Math.abs(y - toCompare.y) < tolerance && Math.abs(z - toCompare.z) < tolerance && (!allFour || Math.abs(w - toCompare.w) < tolerance);
	}
	
	
	inline public function negate():Void
	{
		x *= -1;
		y *= -1;
		z *= -1;
	}
	
	
	inline public function normalize():Float
	{
		var l = length;
		if (l != 0) {	
			x /= l;
			y /= l;
			z /= l;
		}
		return l;
	}
	
	
	inline public function project():Void
	{
		x /= w;
		y /= w;
		z /= w;
	}
	
	
	inline public function scaleBy(s:Float):Void
	{
		x *= s;
		y *= s;
		z *= s;
	}
	
	
	inline public function subtract(a:Vector3D):Vector3D
	{
		return new Vector3D(x - a.x, y - a.y, z - a.z);
	}
	
	
	inline public function toString():String
	{
		return "Vector3D(" + x + ", " + y + ", " + z + ")";
	}
	
	
	
	// Getters & Setters
	
	
	
	/**
	 * @private
	 */
	inline public function nmeGetLength():Float { return Math.sqrt(x * x + y * y + z * z); }
	
	/**
	 * @private
	 */
	inline public function nmeGetLengthSquared():Float { return x * x + y * y + z * z; }
	
	/**
	 * @private
	 */
	inline public static function nmeGetX_AXIS():Vector3D { return new Vector3D(1, 0, 0);	}
	
	/**
	 * @private
	 */
	inline public static function nmeGetY_AXIS():Vector3D { return new Vector3D(0, 1, 0);	}
	
	/**
	 * @private
	 */
	inline public static function nmeGetZ_AXIS():Vector3D { return new Vector3D(0, 0, 1);	}
	
}


#else
typedef Vector3D = flash.geom.Vector3D;
#end