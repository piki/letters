use std::env;

use sdl2::audio::{AudioSpecDesired, AudioSpecWAV};
use std::time::Duration;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        println!("Usage:");
        println!("  {} foo.wav...", args[0]);
        return;
    }
    let sdl = sdl2::init().unwrap();
    let sounds: Vec<_> = args[1..].iter().map(|f|
        AudioSpecWAV::load_wav(f).unwrap()
    ).collect();
    for sound in sounds {
        let audio_len = sound.buffer().len(); // bytes 
        dbg!(&audio_len);
        let desired = AudioSpecDesired{
            freq: Some(sound.freq),
            channels: Some(sound.channels),
            samples: Some(sound.format as u16)
        };
        let audio = sdl.audio().unwrap();
        let sound_queue = audio.open_queue(None, &desired).unwrap();
        sound_queue.queue_audio(sound.buffer()).unwrap();
        sound_queue.resume();
        std::thread::sleep(Duration::from_millis(audio_len as u64 * 1000 / sound.freq as u64 / sound.channels as u64));
    }
}
