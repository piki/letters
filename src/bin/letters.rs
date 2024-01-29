use std::collections::HashMap;
use std::time::Duration;
use sdl2::{ttf::{self, Font}, pixels::Color, event::Event, keyboard::Mod, render::Canvas, video::Window, rect::{Rect, Point}, audio::{AudioSpecWAV, AudioSpecDesired}, Sdl};

fn main() {
    let sdl = sdl2::init().unwrap();
    let mut canvas = init_video(&sdl);

    let ttf = ttf::init().unwrap();
    let font = ttf.load_font("font.ttf", 600).unwrap();

    let sounds = init_audio();

    canvas.set_draw_color(Color::WHITE);
    canvas.clear();
    canvas.present();

    for event in sdl.event_pump().unwrap().wait_iter() {
        match event {
            Event::Quit { timestamp: _ } => { break },
            Event::KeyDown {
                timestamp: _,
                window_id: _,
                keycode,
                scancode: _,
                keymod,
                repeat: _,
            } => {
                if let Some(c) = keycode {
                    if c.name().len() == 1 {
                        let c = c.name().chars().next().unwrap().to_ascii_lowercase();

                        // quit on ^C
                        if c == 'c' && (keymod & (Mod::LCTRLMOD|Mod::RCTRLMOD)) != Mod::NOMOD {
                            break;
                        }

                        // display and play sound for any letter or number
                        if c.is_alphanumeric() {
                            draw_letter(&mut canvas, &font, c);
                            play_sound(&sdl, sounds.get(&c).unwrap());
                        }
                    }
                }
            }
            _ => (),
        };
    }

    println!("quit");
}

fn draw_letter(canvas: &mut Canvas<Window>, font: &Font, c: char) {
    canvas.set_draw_color(Color::WHITE);
    canvas.clear();

    let ci = c as u8 % 7;
    let text_color = Color::RGB(240*(ci&1), 240*((ci>>1)&1), 255*((ci>>2)&1));
    let buf = if c.is_alphabetic() { c.to_ascii_uppercase().to_string() + &c.to_string() } else { c.to_string() };
    let message = font.render(&buf).shaded(text_color, Color::WHITE).unwrap();
    let renderer = canvas.texture_creator();
    let texture = message.as_texture(&renderer).unwrap();
    let dim = canvas.output_size().unwrap();
    let dst = Rect::from_center(Point::new(dim.0 as i32/2, dim.1 as i32/2), message.width(), message.height());
    canvas.copy(&texture, None, dst).unwrap();

    canvas.present();
}

fn init_video(sdl: &Sdl) -> Canvas<Window> {
    let video = sdl.video().unwrap();
    let dm = video.desktop_display_mode(0).unwrap();
    let window = sdl.video().unwrap().window("Letters", dm.w as u32, dm.h as u32).fullscreen().build().unwrap();
    window.into_canvas().build().unwrap()
}

fn init_audio() -> HashMap<char, AudioSpecWAV> {
    let mut sounds: HashMap<char, AudioSpecWAV> = HashMap::default();
    for c in ('a'..='z').chain('0'..='9') {
        let filename = format!("sounds/{c}.wav");
        sounds.insert(c, AudioSpecWAV::load_wav(filename).unwrap());
    }
    println!("Loaded {} sounds", sounds.len());

    sounds
}

fn play_sound(sdl: &Sdl, sound: &AudioSpecWAV) {
    let desired = AudioSpecDesired{
        freq: Some(sound.freq),
        channels: Some(sound.channels),
        samples: Some(sound.format as u16)
    };
    let audio = sdl.audio().unwrap();
    let sound_queue = audio.open_queue(None, &desired).unwrap();
    sound_queue.queue_audio(sound.buffer()).unwrap();
    sound_queue.resume();
    let audio_len = sound.buffer().len(); // bytes 
    std::thread::sleep(Duration::from_millis(audio_len as u64 * 1000 / sound.freq as u64 / sound.channels as u64));
}