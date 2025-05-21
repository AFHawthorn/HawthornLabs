#include "plugin.hpp"
#include <chrono>
#include <ctime>

struct PlanetaryLFOs : Module {
	enum ParamId {
		SPEED_PARAM,
		RESET_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INP,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(TR, 9),
		ENUMS(LFO, 9),
		OUTPUTS_LEN
	};
	enum LightIds {
		ENUMS(LIGHT, 6),
		LIGHTS_LEN
	};

	double mercury = 0.000000131523569023569; 		// was 7603200					
	double venus = 0.0000000515090078953007;		// was 19414080
	double earth = 0.0000000316924262707395;		// was 31553280
	double mars = 0.0000000168472693945765;			// was 59356800
	double jupiter = 0.00000000267237914432558;		// was 374198400
	double saturn = 0.00000000107695859998828;		// was 928540800
	double uranus = 0.000000000378373731539902;		// was 2642889600
	double neptune = 0.000000000193546389198563;	// was 5166720000
	double pluto = 0.000000000127805588273786;		// was 7824384000

	long timeSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	long planetDur[9] = {7603200, 19414080, 31553280, 59356800, 374198400, 928540800, 2642889600, 5166720000, 7824384000}; 						// duration of each planet's year in seconds, used against timeSinceEpoch to figure out starting phase.
	double phase[9];

	// float realtime = 1.f;
	// float inADay = 365.2f;
	// float inAnHour = 8764.8f;
	// float inAMin = 525888.f;
	// float inASec = 31553280.f;
	// float inAMs = 31553280000.f;

	int counter = 2;
	int speedKnob = 1;
	float timeCompression = 1.f;
	float _gain = 5.f;
	float _2PI = 2.f * M_PI;

	dsp::PulseGenerator trig[9];

	double planet[9] = {mercury, venus, earth, mars, jupiter, saturn, uranus, neptune, pluto};
	double freq[9] = {mercury, venus, earth, mars, jupiter, saturn, uranus, neptune, pluto};

	PlanetaryLFOs() {
		for (int i = 0; i < 9; i ++){phase[i] = ((double)(timeSinceEpoch % planetDur[i]) / (double)planetDur[i])*_2PI;};
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPEED_PARAM, 1, 1100, 1, "Time Compression");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset all LFOs");
		configInput(RESET_INP, "Reset Trig");
		configOutput(TR + 0, "Mercury Trig");
		configOutput(TR + 1, "Venus Trig");
		configOutput(TR + 2, "Earth Trig");
		configOutput(TR + 3, "Mars Trig");
		configOutput(TR + 4, "Jupiter Trig");
		configOutput(TR + 5, "Saturn Trig");
		configOutput(TR + 6, "Uranus Trig");
		configOutput(TR + 7, "Neptune Trig");
		configOutput(TR + 8, "Pluto Trig");
		configOutput(LFO + 0, "Mercury LFO");
		configOutput(LFO + 1, "Venus LFO");
		configOutput(LFO + 2, "Earth LFO");
		configOutput(LFO + 3, "Mars LFO");
		configOutput(LFO + 4, "Jupiter LFO");
		configOutput(LFO + 5, "Saturn LFO");
		configOutput(LFO + 6, "Uranus LFO");
		configOutput(LFO + 7, "Neptune LFO");
		configOutput(LFO + 8, "Pluto LFO");
	}

	void process(const ProcessArgs& args) override {
		if (counter % 2 == 0){	// LFO's are slow, so run every other sample (could do half and half)
			for (int i = 0; i < 9; i ++){
				freq[i] = planet[i] * timeCompression;
				double phase_increment = _2PI * freq[i] / args.sampleRate;		// calc phase increment
				phase[i] += phase_increment;									// push osc forward 2 steps
				if (phase[i] >= _2PI){
					phase[i] -= _2PI;
					trig[i].trigger(0.01f);
				};
				if (outputs[LFO + i].isConnected()) {
					double sine_output = _gain * sin(phase[i]);
					outputs[LFO + i].setVoltage(sine_output);
				};
				if (outputs[TR + i].isConnected()) {
					outputs[TR + i].setVoltage(trig[i].process(args.sampleTime) ? 10.0f : 0.0f);
				};
			};
		};
		if (counter % 8 == 0){	// check speed knob every 8 samples
			if (speedKnob != params[SPEED_PARAM].getValue())
				updateTimeCompression();
			if (params[RESET_PARAM].getValue() > 0.5f || inputs[RESET_INP].getVoltage() > 0.1f)
				resetLFOs();
		};
		counter++;
		if (counter > args.sampleRate)
			counter = 1;
	}

	void updateTimeCompression(){
		speedKnob = params[SPEED_PARAM].getValue();
		if (speedKnob <= 2){
			timeCompression = 1.f;
			lights[LIGHT + 0].setSmoothBrightness(1.f, 0.1f);
		} else if (speedKnob > 2 && speedKnob <=218){
			timeCompression = scaleKnobValue(speedKnob, 1, 218, 1.f, 365.2f);
			refreshLights();
		} else if (speedKnob > 218 && speedKnob <= 222){
			timeCompression = 365.2f;
			lights[LIGHT + 1].setSmoothBrightness(1.f, 0.1f);
		} else if (speedKnob > 222 && speedKnob <= 428){
			timeCompression = scaleKnobValue(speedKnob, 222, 428, 365.2f, 8764.8f);
			refreshLights();
		} else if (speedKnob > 428 && speedKnob <= 432){
			timeCompression = 8764.8f;
			lights[LIGHT + 2].setSmoothBrightness(1.f, 0.1f);
		} else if (speedKnob > 432 && speedKnob <= 648){
			timeCompression = scaleKnobValue(speedKnob, 432, 648, 8764.8f, 525888.f);
			refreshLights();
		} else if (speedKnob > 648 && speedKnob <= 652){
			timeCompression = 525888.f;
			lights[LIGHT + 3].setSmoothBrightness(1.f, 0.1f);
		} else if (speedKnob > 652 && speedKnob <= 878){
			timeCompression = scaleKnobValue(speedKnob, 652, 878, 525888.f, 31553280.f);
			refreshLights();
		} else if (speedKnob > 878 && speedKnob <= 882){
			timeCompression = 31553280.f;
			lights[LIGHT + 4].setSmoothBrightness(1.f, 0.1f);
		} else if (speedKnob > 882 && speedKnob < 1097){
			timeCompression = scaleKnobValue(speedKnob, 882, 1097, 31553280.f, 31553280000.f);
			refreshLights();
		} else if (speedKnob >= 1097){
			timeCompression = 31553280000.f;
			lights[LIGHT + 5].setSmoothBrightness(1.f, 0.1f);
		} else {
			timeCompression = 1.f;
			refreshLights();
		};
	}

	void resetLFOs(){
		memset(phase, 0.0, sizeof(phase));
	}

	float scaleKnobValue(float speedKnob, int knobMin, int knobMax, float scaleMin, float scaleMax){
		float knobPercent = (speedKnob - knobMin)/(knobMax - knobMin);
		return (knobPercent) * (scaleMax - scaleMin) + scaleMin;
	}

	void refreshLights(){
		for(int i = 0; i<6; i++){
			lights[LIGHT + i].setSmoothBrightness(0.f, 0.1f);
		};
	}
};

struct PlanetaryLFOsWidget : ModuleWidget {
	PlanetaryLFOsWidget(PlanetaryLFOs* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/PlanetaryLFOs.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.55, 73.104)), module, PlanetaryLFOs::SPEED_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(61.179, 112.134)), module, PlanetaryLFOs::RESET_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(61.179, 101.694)), module, PlanetaryLFOs::RESET_INP));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.108, 25.624)), module, PlanetaryLFOs::TR + 0));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.626, 32.546)), module, PlanetaryLFOs::TR + 1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.143, 41.415)), module, PlanetaryLFOs::TR + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.661, 54.254)), module, PlanetaryLFOs::TR + 3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.179, 73.104)), module, PlanetaryLFOs::TR + 4));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.661, 86.148)), module, PlanetaryLFOs::TR + 5));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.143, 95.333)), module, PlanetaryLFOs::TR + 6));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.626, 101.694)), module, PlanetaryLFOs::TR + 7));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.108, 106.219)), module, PlanetaryLFOs::TR + 8));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.108, 36.064)), module, PlanetaryLFOs::LFO + 0));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.626, 42.985)), module, PlanetaryLFOs::LFO + 1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.143, 51.854)), module, PlanetaryLFOs::LFO + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.661, 64.693)), module, PlanetaryLFOs::LFO + 3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.179, 83.543)), module, PlanetaryLFOs::LFO + 4));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.661, 96.587)), module, PlanetaryLFOs::LFO + 5));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.143, 105.772)), module, PlanetaryLFOs::LFO + 6));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.626, 112.134)), module, PlanetaryLFOs::LFO + 7));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.108, 116.658)), module, PlanetaryLFOs::LFO + 8));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(17.00, 78.804)), module, PlanetaryLFOs::LIGHT + 0));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(14.00, 73.104)), module, PlanetaryLFOs::LIGHT + 1));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(17.00, 67.50)), module, PlanetaryLFOs::LIGHT + 2));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.75, 67.50)), module, PlanetaryLFOs::LIGHT + 3));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(27.1, 73.104)), module, PlanetaryLFOs::LIGHT + 4));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.75, 78.804)), module, PlanetaryLFOs::LIGHT + 5));
	}
};

Model* modelPlanetaryLFOs = createModel<PlanetaryLFOs, PlanetaryLFOsWidget>("PlanetaryLFOs");