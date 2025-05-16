# Hawthorn Labs
is a series of modules for VCV Rack.  Ok, well, currently only ONE module, and that module is called...
## Planetary LFOs
A module for VCV Rack that uses the revolutions of the planets in our solar system as a basis for LFO rates.  For the purposes of this module, Pluto is included as a planet.
A trigger output is also provided that sends a +10v pulse every time a revolution is complete (every "year").

The LFOs are currently instantiated as if the planets were magically aligned all at phase = 0 on 1/1/1970 and calculates their positions at the current second.

The reset button and trigger input reset all the phases to zero.  This cannot (currently) be undone.

- The Speed knob has 6 lights on it:
    - Fully "down", CCW, the LFO's are produced in realtime
    - The second light represents an Earth year in a day (a time compression of 365.25x)
    - Third light: an Earth year in an hour
    - Fourth light: an Earth year in a minute
    - Fifth light: an Earth year in a second
    - Sixth light: an Earth year in a millisecond (some LFOs are well into audio-rate by this point)

### Planetary Year Data
- Mercury's year lasts 88 Earth-days
- Venus' year is 224.7 Earth-days
- Earth's year is 365.25 Earth-days
- Mars' year is 687 Earth-days
- Jupiter's year is 4331 Earth-days
- Saturn's year is 10,747 Earth-days
- Uranus' year is 30,589 Earth-days
- Neptune's year is 59,800 Earth-days
- and Pluto's year is 90,560 Earth-days

I hope you enjoy! Feedback and bug reports (and [donations](https://www.paypal.me/afhawthorn)!)are very welcome.