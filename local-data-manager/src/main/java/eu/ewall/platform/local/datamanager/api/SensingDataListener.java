/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.local.datamanager.api;

import eu.ewall.platform.commons.datamodel.measure.AccelerometerMeasurement;
import eu.ewall.platform.commons.datamodel.measure.IndoorMeasurement;
import eu.ewall.platform.commons.datamodel.measure.MattressPressureSensing;
import eu.ewall.platform.commons.datamodel.sensing.AppliancePowerSensing;
import eu.ewall.platform.commons.datamodel.sensing.SpeakerSensing;
import eu.ewall.platform.commons.datamodel.sensing.VisualSensing;
import eu.ewall.platform.commons.datamodel.sensing.VitalsSensing;

/**
 * The listener interface for receiving sensingData events.
 * The class that is interested in processing a sensingData
 * event implements this interface, and the object created
 * with that class is registered with a component using the
 * component's <code>addSensingDataListener<code> method. When
 * the sensingData event occurs, that object's appropriate
 * method is invoked.
 *
 * @see SensingDataEvent
 * @author emirmos
 */
public interface SensingDataListener {

	/**
	 * Indoor measurement data event.
	 *
	 * @param measurement the measurement
	 * @return true, if successful
	 */
	public boolean indoorMeasurementDataEvent(IndoorMeasurement measurement);

	/**
	 * Activity sensing data event.
	 *
	 * @param accMeasurement the acc measurement
	 * @return true, if successful
	 */
	public boolean activitySensingDataEvent(
			AccelerometerMeasurement accMeasurement);

	/**
	 * Appliance power sensing data event.
	 *
	 * @param appliancePowerSensing the appliance power sensing
	 * @return true, if successful
	 */
	public boolean appliancePowerSensingDataEvent(
			AppliancePowerSensing appliancePowerSensing);

	/**
	 * Furnitire sensing data event.
	 *
	 * @param pressureSensing the pressure sensing
	 * @return true, if successful
	 */
	public boolean furnitireSensingDataEvent(
			MattressPressureSensing pressureSensing);

	/**
	 * Speaker sensing data event.
	 *
	 * @param speakerSensing the speaker sensing
	 * @return true, if successful
	 */
	public boolean speakerSensingDataEvent(SpeakerSensing speakerSensing);

	/**
	 * Visual sensing data event.
	 *
	 * @param visualSensing the visual sensing
	 * @return true, if successful
	 */
	public boolean visualSensingDataEvent(VisualSensing visualSensing);

	/**
	 * Vitals sensing data event.
	 *
	 * @param vitalsSensing the vitals sensing
	 * @return true, if successful
	 */
	public boolean vitalsSensingDataEvent(VitalsSensing vitalsSensing);

}
