package installers;


import haxe.Int32;
import haxe.io.Bytes;
import haxe.SHA1;
import neko.FileSystem;
import neko.io.File;
import neko.io.Path;
import neko.Lib;
import neko.Sys;
import data.Asset;
import neko.zip.Writer;
import nme.utils.ByteArray;

import format.swf.Data;
import format.swf.Constants;
import format.mp3.Data;
import format.wav.Data;

import nme.text.Font;


class FlashInstaller extends InstallerBase {
	
	
	override function build ():Void {
		
		var destination:String = buildDirectory + "/flash/bin/";
		mkdir (destination);
		
		recursiveCopy (NME + "/tools/command-line/haxe", buildDirectory + "/flash/haxe");
		recursiveCopy (NME + "/tools/command-line/flash/hxml", buildDirectory + "/flash/haxe");
		recursiveCopy (NME + "/tools/command-line/flash/haxe", buildDirectory + "/flash/haxe");
		
		var hxml:String = buildDirectory + "/flash/haxe/" + (debug ? "debug" : "release") + ".hxml";
		
		runCommand ("", "haxe", [ hxml ] );
		
		var file = defines.get ("APP_FILE") + ".swf";
		var input = neko.io.File.read (destination + "/" + file, true);
		var reader = new format.swf.Reader (input);
		var swf = reader.read ();
		input.close();
		
		var new_tags = new Array <SWFTag> ();
		var inserted = false;
		
		for (tag in swf.tags) {
			
			var name = Type.enumConstructor(tag);
			//trace(name);
			//if (name=="TSymbolClass") trace(tag);
			
			if (name == "TShowFrame" && !inserted && assets.length > 0) {
				
				new_tags.push (TShowFrame);
				
				for(asset in assets) {
					
					if (toSwf (asset, new_tags)) {
						
						inserted = true;
						
					}
					
				}
				
			}
			
			new_tags.push (tag);
			
		}

		if (inserted) {
			
			swf.tags = new_tags;
			var output = neko.io.File.write (destination + "/" + file, true);
			var writer = new format.swf.Writer (output);
			writer.write (swf);
			output.close ();
			
		}
		
		if (targetFlags.exists ("web")) {
			
			recursiveCopy (NME + "/tools/command-line/flash/templates/web", buildDirectory + "/flash/bin");
			
		} else if (targetFlags.exists ("chrome")) {
			
			recursiveCopy (NME + "/tools/command-line/flash/templates/chrome", buildDirectory + "/flash/bin");
			
			getIcon (16, buildDirectory + "/flash/bin/icon_16.png");
			getIcon (128, buildDirectory + "/flash/bin/icon_128.png");
			
			//compressToZip (buildDirectory + "/flash/bin/" + defines.get ("APP_FILE") + ".crx");
			
		} else if (targetFlags.exists ("opera")) {
			
			recursiveCopy (NME + "/tools/command-line/flash/templates/opera", buildDirectory + "/flash/bin");
			
			getIcon (16, buildDirectory + "/flash/bin/icon_16.png");
			getIcon (32, buildDirectory + "/flash/bin/icon_32.png");
			getIcon (64, buildDirectory + "/flash/bin/icon_64.png");
			getIcon (128, buildDirectory + "/flash/bin/icon_128.png");
			
			compressToZip (buildDirectory + "/flash/bin/" + defines.get ("APP_FILE") + ".wgt");
			
		}
		
	}
	
	
	private function compressToZip (path:String):Void {
		
		var files = new Array <Dynamic> ();
		
		var directory = Path.directory (path);
		
		for (file in FileSystem.readDirectory (directory)) {
			
			if (Path.extension (file) != "zip" && Path.extension (file) != "crx" && Path.extension (file) != "wgt") {
				
				var name = file;
				//var date = FileSystem.stat (directory + "/" + file).ctime;
				var date = Date.now ();
				
				var input = File.read (directory + "/" + file, true);
				var data = input.readAll ();
				input.close ();
				
				files.push ( { fileName: name, fileTime: date, data: data } );
				
			}
			
		}
		
		var output = File.write (path, true);
		
		if (Path.extension (path) == "crx") {
			
			var input = File.read (defines.get ("KEY_STORE"), true);
			var publicKey:Bytes = input.readAll ();
			input.close ();
			
			var signature = SHA1.encode ("this isn't working");
			
			output.writeString ("Cr24"); // magic number
			output.writeInt32 (Int32.ofInt (2)); // CRX file format version
			output.writeInt32 (Int32.ofInt (publicKey.length)); // public key length
			output.writeInt32 (Int32.ofInt (signature.length)); // signature length
			output.writeBytes (publicKey, 0, publicKey.length);
			output.writeString (signature);
			
			//output.writeBytes (); // public key contents "The contents of the author's RSA public key, formatted as an X509 SubjectPublicKeyInfo block. "
			//output.writeBytes (); // "The signature of the ZIP content using the author's private key. The signature is created using the RSA algorithm with the SHA-1 hash function."
			
		}
		
		Writer.writeZip (output, files, 1);
		output.close ();
		
	}
	
	
	static var swfAssetID = 1000;
	
	function nextAssetID () {
		
		return swfAssetID++;
		
	}
   

	public function toSwf (inAsset:Asset, outTags:Array<SWFTag>) {
		
		var embed = inAsset.embed;
		var name = inAsset.sourcePath;
		var type = inAsset.type;
		var flatName = inAsset.flatName;
		
		if (!embed) {
			
			return false;
			
		}
		
		var cid = nextAssetID ();
		
		if (type == "music" || type == "sound") {
			
			var src = name;
			var ext = neko.io.Path.extension (src);
			
			if (ext != "mp3" && ext != "wav") {
				
				for (e in ["wav", "mp3"]) {
					
					src = name.substr (0, name.length - ext.length) + e;
					
					if (neko.FileSystem.exists (src)) {
						
						break;
						
					}
					
				}
				
			}
			
			if (!neko.FileSystem.exists (src)) {
				
				throw "Could not find mp3/wav source: " + src;
				
			}
			
			var ext = neko.io.Path.extension (src);
			
			var input = neko.io.File.read (src, true);
			
			if (ext == "mp3") {
				
				// Code lifted from "samhaxe"
				var r = new format.mp3.Reader (input);
				var mp3 = r.read ();
				
				if (mp3.frames.length == 0) {
					
					throw "No frames found in mp3: " + src;
					
				}
				
				// Guess about the format based on the header of the first frame found
				var fr0 = mp3.frames[0];
				var hdr0 = fr0.header;
				
				// Verify Layer3-ness
				if (hdr0.layer != Layer.Layer3) {
					
					throw "Only Layer-III mp3 files are supported by flash. File " + src + " is: " + format.mp3.Tools.getFrameInfo (fr0);
					
				}

				// Check sampling rate
				var flashRate = switch (hdr0.samplingRate) {
					
					case SR_11025: SR11k;
					case SR_22050: SR22k;
					case SR_44100: SR44k;
					default:
						throw "Only 11025, 22050 and 44100 Hz mp3 files are supported by flash. File " + src + " is: " + format.mp3.Tools.getFrameInfo (fr0);
					
				}

				var isStereo = switch (hdr0.channelMode) {
					
					case Stereo, JointStereo, DualChannel: true;
					case Mono: false;
					
				}
				
				// Should we do this? For now, let's do.
				var write_id3v2 = true;
				
				var rawdata = new haxe.io.BytesOutput ();
				(new format.mp3.Writer (rawdata)).write (mp3, write_id3v2);
				var dataBytes = rawdata.getBytes ();
				
				var snd = {
					
					sid : cid,
					format : SFMP3,
					rate : flashRate,
					is16bit : true,
					isStereo : isStereo,
					samples : haxe.Int32.ofInt (mp3.sampleCount),
					data : SDMp3 (0, dataBytes)
					
				};
				
				outTags.push (TSound (snd));
				
			} else {
				
				var r = new format.wav.Reader (input);
				var wav = r.read ();
				var hdr = wav.header;
				
				if (hdr.format != WF_PCM) {
					
					throw "Only PCM (uncompressed) wav files can be imported.";
					
				}
				
				// Check sampling rate
				var flashRate = switch (hdr.samplingRate) {
					
					case  5512: SR5k;
					case 11025: SR11k;
					case 22050: SR22k;
					case 44100: SR44k;
					default:
						throw "Only 5512, 11025, 22050 and 44100 Hz wav files are supported by flash. Sampling rate of '" + src + "' is: " + hdr.samplingRate;
					
				}

				var isStereo = switch (hdr.channels) {
					
					case 1: false;
					case 2: true;
					default: 
						throw "Number of channels should be 1 or 2, but for '" + src + "' it is " + hdr.channels;
					
				}

				var is16bit = switch (hdr.bitsPerSample) {
					
					case 8: false;
					case 16: true;
					default: 
						throw "Bits per sample should be 8 or 16, but for '" + src + "' it is " + hdr.bitsPerSample;
					
				}
				
				var sampleCount = Std.int (wav.data.length / (hdr.bitsPerSample / 8));
				
				var snd:format.swf.Sound = {
					
					sid : cid,
					format : SFLittleEndianUncompressed,
					rate : flashRate,
					is16bit : is16bit,
					isStereo : isStereo,
					samples : haxe.Int32.ofInt (sampleCount),
					data : SDRaw (wav.data)
					
				}
				
				outTags.push (TSound (snd));
				
			}
			
			input.close ();
			
		} else if (type == "image") {
			
			var src = name;
			var ext = neko.io.Path.extension (src).toLowerCase ();
			
			if (ext == "jpg" || ext == "png") {
				
				var bytes:haxe.io.Bytes;
				try { bytes = neko.io.File.read (src, true).readAll (); }
				catch (e:Dynamic) { throw "Could not load image file: " + src; }
				
				outTags.push (TBitsJPEG (cid, JDJPEG2 (bytes)));
				
			} else {
				
				throw ("Unknown image type:" + src );
				
			}
			
		} else if (type == "font") {
			
			// More code ripped off from "samhaxe"
			var src = name;
			var font_name = neko.io.Path.withoutExtension (name);
			var font = nme.text.Font.load (src);
			
			var glyphs = new Array <Font2GlyphData> ();
			var glyph_layout = new Array <FontLayoutGlyphData> ();
			
			for (native_glyph in font.glyphs) {
				
				if (native_glyph.char_code > 65535) {
					
					neko.Lib.println("Warning: glyph with character code greater than 65535 encountered ("+ native_glyph.char_code+"). Skipping...");
					continue;
					
				}
				
				var shapeRecords = new Array <ShapeRecord> ();
				var i:Int = 0;
				var styleChanged:Bool = false;

				while (i < native_glyph.points.length) {
					
					var type = native_glyph.points[i++];
					
					switch (type) {
						
						case 1: // Move
							
							var dx = native_glyph.points[i++];
							var dy = native_glyph.points[i++];
							shapeRecords.push( SHRChange({
								moveTo: {dx: dx, dy: -dy},
								// Set fill style to 1 in first style change record
								// Required by DefineFontX
								fillStyle0: if (!styleChanged) {idx: 1} else null,
								fillStyle1: null,
								lineStyle:  null,
								newStyles:  null
							}));
							styleChanged = true;
						
						case 2: // LineTo
							
							var dx = native_glyph.points[i++];
							var dy = native_glyph.points[i++];
							shapeRecords.push (SHREdge(dx, -dy));
						
						case 3: // CurveTo
							var cdx = native_glyph.points[i++];
							var cdy = native_glyph.points[i++];
							var adx = native_glyph.points[i++];
							var ady = native_glyph.points[i++];
							shapeRecords.push (SHRCurvedEdge(cdx, -cdy, adx, -ady));
						
						default:
							throw "Invalid control point type encountered! (" + type + ")";
						
					}
					
				}
				
				shapeRecords.push (SHREnd);
				
				glyphs.push({
					charCode: native_glyph.char_code,
					shape: {
						shapeRecords: shapeRecords
					} 
				});
				
				glyph_layout.push({
					advance: native_glyph.advance,
					bounds: {
						left:    native_glyph.min_x,
						right:   native_glyph.max_x,
						top:    -native_glyph.max_y,
						bottom: -native_glyph.min_y,
					}
				});
				
			}
			
			var kerning = new Array <FontKerningData> ();
			
			if (font.kerning != null) {
				
				for (k in font.kerning) {
					
					kerning.push ({
						charCode1:  k.left_glyph,
						charCode2:  k.right_glyph,
						adjust:     k.x,
					});
					
				}
				
			}
			
			var swf_em = 1024 * 20;
			var ascent = Math.ceil (font.ascend * swf_em / font.em_size);
			var descent = -Math.ceil (font.descend * swf_em / font.em_size);
			var leading = Math.ceil ((font.height - font.ascend + font.descend) * swf_em / font.em_size);
			var language = LangCode.LCNone;
			
			outTags.push (TFont (cid, FDFont3 ({
				shiftJIS:   false,
				isSmall:    false,
				isANSI:     false,
				isItalic:   font.is_italic,
				isBold:     font.is_bold,
				language:   language,
				name:       font_name,
				glyphs:     glyphs,
				layout: {
					ascent:     ascent,
					descent:    descent,
					leading:    leading,
					glyphs:     glyph_layout,
					kerning:    kerning
				}
			})) );
			
		} else {
			
			var bytes = neko.io.File.getBytes (name);
			outTags.push (TBinaryData (cid, bytes));
			
		}
		
		outTags.push (TSymbolClass ( [ { cid:cid, className:"NME_" + flatName } ] ));
		
		return true;
		
	}
	
	
	override function generateContext ():Void {
		
		super.generateContext ();
		
		if (targetFlags.exists ("opera")) {
			
			var packageName = defines.get ("APP_PACKAGE");
			
			context.APP_PACKAGE_HOST = packageName.substr (0, packageName.lastIndexOf ("."));
			context.APP_PACKAGE_NAME = packageName.substr (packageName.lastIndexOf (".") + 1);
			
			var currentDate = Date.now ();
			
			var revisionDate = currentDate.getFullYear () + "-";
			
			var month = currentDate.getMonth ();
			
			if (month < 10) {
				
				revisionDate += "0" + month;
				
			} else {
				
				revisionDate += month;
				
			}
			
			var day = currentDate.getDate ();
			
			if (day < 10) {
				
				revisionDate += "-0" + day;
				
			} else {
				
				revisionDate += "-" + day;
				
			}
			
			context.REVISION_DATE = revisionDate;
			
		}
		
	}
	
	
	private function getIcon (size:Int, targetPath:String):Void {
		
		var icon = icons.findIcon (size, size);
		
		if (icon != "") {
			
			copyIfNewer (icon, targetPath);
			
		} else {
			
			icons.updateIcon (size, size, targetPath);
			
		}
		
	}
	
	
	override function run ():Void {
		
		var destination:String = buildDirectory + "/flash/bin";
		var player:String;
		
		if (defines.exists ("SWF_PLAYER")) {
			
			player = defines.get ("SWF_PLAYER");
			
		} else {
			
			player = Sys.getEnv ("FLASH_PLAYER_EXE");
			
		}
		
		if (player == null || player == "") {
			
			var dotSlash:String = "./";
			
			if (defines.exists ("windows")) {
				
				runCommand (destination, ".\\" + defines.get ("APP_FILE") + ".swf", []);
				
			} else {
				
				runCommand (destination, "open", [ defines.get ("APP_FILE") + ".swf" ]);
				
			}
			
		} else {
			
			runCommand (destination, player, [ defines.get ("APP_FILE") + ".swf" ]);
			
		}
		
	}
	
	
	override function update ():Void {
		
		var destination:String = buildDirectory + "/flash/bin";
		
		for (asset in assets) {
			
			if (!asset.embed) {
				
				mkdir (Path.directory (destination + asset.targetPath));
				copyIfNewer (asset.sourcePath, destination + asset.targetPath);
				
			}
			
		}
		
	}
	
	
}

