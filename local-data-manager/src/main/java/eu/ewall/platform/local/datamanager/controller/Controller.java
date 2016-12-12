/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.local.datamanager.controller;

import java.util.ArrayList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import eu.ewall.platform.commons.datamodel.measure.AccelerometerMeasurement;
import eu.ewall.platform.commons.datamodel.measure.IndoorMeasurement;
import eu.ewall.platform.commons.datamodel.measure.MattressPressureSensing;
import eu.ewall.platform.commons.datamodel.sensing.AppliancePowerSensing;
import eu.ewall.platform.commons.datamodel.sensing.SpeakerSensing;
import eu.ewall.platform.commons.datamodel.sensing.VisualSensing;
import eu.ewall.platform.commons.datamodel.sensing.VitalsSensing;
import eu.ewall.platform.local.datamanager.api.SensingDataListener;

/**
 * The Class Controller.
 * 
 * @author emirmos
 */
public class Controller {
	
	/** The logger. */
	Logger logger = LoggerFactory.getLogger(Controller.class);
	
	/** The listeners. */
	private List<SensingDataListener> listeners = new ArrayList<SensingDataListener>(); 

	
	/**
	 * Adds the listener.
	 *
	 * @param listener the listener
	 * @return true, if successful
	 */
	public boolean addListener(SensingDataListener listener) {
		if (listener != null) {
			listeners.add(listener);
			logger.debug("Added new sensing data listener of class {}", listener.getClass());
			return true;
		}
		logger.debug("Adding sensing data listener was not successful. Listener is null");
		return false;
		
	}
	
	
	/**
	 * Removes the listener.
	 *
	 * @param listener the listener
	 * @return true, if successful
	 */
	public boolean removeListener(SensingDataListener listener) {
		logger.debug("Removing sensing data listener of class {}", listener.getClass());
		return listeners.remove(listener);
	}
	
	
	/**
	 * Process indoor measurement data.
	 *
	 * @param measurement the measurement
	 * @return true, if successful
	 */
	public boolean processIndoorMeasurementData(IndoorMeasurement measurement) {

		for (SensingDataListener listener : listeners) {
			listener.indoorMeasurementDataEvent(measurement);
		}
		
		return true;
	}

	/**
	 * Process activity sensing data.
	 *
	 * @param accMeasurement the acc measurement
	 * @return true, if successful
	 */
	public boolean processActivitySensingData(
			AccelerometerMeasurement accMeasurement) {
		for (SensingDataListener listener : listeners) {
			listener.activitySensingDataEvent(accMeasurement);
		}
		return true;
	}

	/**
	 * Process appliance power sensing data.
	 *
	 * @param appliancePowerSensing the appliance power sensing
	 * @return true, if successful
	 */
	public boolean processAppliancePowerSensingData(
			AppliancePowerSensing appliancePowerSensing) {
		for (SensingDataListener listener : listeners) {
			listener.appliancePowerSensingDataEvent(appliancePowerSensing);
		}
		return true;
	}

	/**
	 * Process furnitire sensing data.
	 *
	 * @param pressureSensing the pressure sensing
	 * @return true, if successful
	 */
	public boolean processFurnitireSensingData(
			MattressPressureSensing pressureSensing) {
		for (SensingDataListener listener : listeners) {
			listener.furnitireSensingDataEvent(pressureSensing);
		}
		return true;
	}

	/**
	 * Process speaker sensing data.
	 *
	 * @param speakerSensing the speaker sensing
	 * @return true, if successful
	 */
	public boolean processSpeakerSensingData(SpeakerSensing speakerSensing) {
		for (SensingDataListener listener : listeners) {
			listener.speakerSensingDataEvent(speakerSensing);
		}
		return true;
	}

	/**
	 * Process visual sensing data.
	 *
	 * @param visualSensing the visual sensing
	 * @return true, if successful
	 */
	public boolean processVisualSensingData(VisualSensing visualSensing) {
		for (SensingDataListener listener : listeners) {
			listener.visualSensingDataEvent(visualSensing);
		}
		return true;
	}

	/**
	 * Process vitals sensing data.
	 *
	 * @param vitalsSensing the vitals sensing
	 * @return true, if successful
	 */
	public boolean processVitalsSensingData(VitalsSensing vitalsSensing) {
		
		for (SensingDataListener listener : listeners) {
			listener.vitalsSensingDataEvent(vitalsSensing);
		}
		return true;
	}

}
