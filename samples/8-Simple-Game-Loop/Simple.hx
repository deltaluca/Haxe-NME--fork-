import nme.Manager;
import nme.Timer;
import nme.Point;
import nme.TTF;

class Simple
{
	static var mainObject : Simple;
	var running : Bool;
	
	static function main()
	{
		mainObject = new Simple();
	}
	
	public function new()
	{
		var mng : Manager = new Manager( 200, 200, "Simple Application", false, "ico.gif" );
			
		var fps : Float;
		var prevTime : Float = 0.0;
		var curTime : Float;
		running = true;
                var text = new TTF("FPS","ARIAL.TTF", 12, 0xffffff, 0x000000, 100, new Point(15,15));
		while (running)
		{
			mng.events();
			switch mng.getEventType()
			{
				case et_keydown:
					processKeys( mng.lastKey(), true );
				case et_quit:
					running = false;
				default:
			}
			
			curTime = Timer.getCurrent();
			fps = 1000.00 / (curTime - prevTime);
			prevTime = curTime;
			
			mng.clear( 0x000000 );
			text.text = Std.string( fps );
                        text.draw();
			mng.flip();
		}
		mng.close();
	}
	
	public function processKeys( key, pressed : Bool )
	{
		switch key
		{
			case 27:
				running = false;
			default:
				neko.Lib.print( key );
		}
	}
}