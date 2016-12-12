package eu.ewall.platform.local.reasoners;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.databind.ObjectMapper;

import eu.ewall.platform.commons.datamodel.measure.AccelerometerMeasurement;
import eu.ewall.platform.commons.datamodel.measure.ConstantQuantityMeasureType;
import eu.ewall.platform.commons.datamodel.measure.IndoorMeasurement;
import eu.ewall.platform.commons.datamodel.measure.MattressPressureSensing;
import eu.ewall.platform.commons.datamodel.message.Notification;
import eu.ewall.platform.commons.datamodel.message.NotificationContentFeedbackMsg;
import eu.ewall.platform.commons.datamodel.message.NotificationContentMsg;
import eu.ewall.platform.commons.datamodel.message.NotificationPriority;
import eu.ewall.platform.commons.datamodel.message.NotificationType;
import eu.ewall.platform.commons.datamodel.profile.preferences.SystemPreferences;
import eu.ewall.platform.commons.datamodel.sensing.AppliancePowerSensing;
import eu.ewall.platform.commons.datamodel.sensing.SpeakerSensing;
import eu.ewall.platform.commons.datamodel.sensing.VisualSensing;
import eu.ewall.platform.commons.datamodel.sensing.VitalsSensing;
import eu.ewall.platform.local.datamanager.LocalDataManager;
import eu.ewall.platform.local.datamanager.api.SensingDataListener;
import eu.ewall.platform.local.reasoners.properties.PropertyUtils;

public class MainReasoner implements SensingDataListener {
	
	Logger logger = LoggerFactory.getLogger(MainReasoner.class);
	LocalDataManager localDataManager;
	ObjectMapper mapper;
	private SystemPreferences systemPreferences;
	private Properties threshold = new Properties();
	private Map<String,Properties> languagesMap;
	private Date lastMeasurement;
	private boolean osWarningSent = false;
	private boolean osAlarmSent = false;
	private Date osWarningTimer;
	private Date osAlarmTimer;
	
	public MainReasoner(LocalDataManager localDataManager) {
		super();
		this.localDataManager = localDataManager;
		 mapper = new ObjectMapper();
		 threshold = PropertyUtils.getThresholds();
		 languagesMap = PropertyUtils.loadAllLanguages();
		 lastMeasurement = new Date();
		 osWarningTimer = new Date();
		 osAlarmTimer = new Date();
		 //load all languages here
		 
	}
	
	public boolean startup() {
		localDataManager.registerListener(this);
		systemPreferences = localDataManager.getUserSystemPreferences();
		return true;
	}
	
	public boolean shutdown() {
		if (localDataManager != null)
			localDataManager.deregisterListener(this);
		return true;
	}
	
	@Override
	public boolean indoorMeasurementDataEvent(IndoorMeasurement measurement) {
		// TODO Auto-generated method stub
		Properties msg = new Properties();
		
		if (systemPreferences != null) {
			logger.info("User prefered language = "+systemPreferences.getPreferredLanguage());
			logger.info("User secondary language = "+systemPreferences.getSecondaryLanguage());
			msg = languagesMap.get(systemPreferences.getPreferredLanguage());
		} else {
			msg = languagesMap.get("en");
		}
		if(measurement.getConstantQuantityMeasureType() == ConstantQuantityMeasureType.CARBON_MONOXIDE_MEASUREMENT) {
			if(Integer.valueOf(measurement.getMeasuredValue()) > Integer.parseInt(threshold.getProperty("gas.co.alert"))) {
				//ALARM
				try {
					Notification notification = new Notification();
					NotificationContentMsg content = new NotificationContentMsg();
					List<NotificationContentFeedbackMsg> feedback = 
							new ArrayList<NotificationContentFeedbackMsg>();
					
					NotificationContentFeedbackMsg feed1 = new NotificationContentFeedbackMsg();
						feed1.setType("button");
						feed1.setLabel(msg.getProperty("gas.feedback.label"));
						feed1.setUrl("");
					
					feedback.add(feed1);
					
					
					notification.setTitle(msg.getProperty("gas.notification.title.alarm"));
					
					content.setType(msg.getProperty("gas.content.type"));
					content.setSubtype(msg.getProperty("gas.co.content.title.alarm"));
					content.setTitle(msg.getProperty("gas.co.content.title.alarm"));
					content.setMotivational(msg.getProperty("gas.co.content.motivational"));
					content.setSuggestion(msg.getProperty("gas.co.content.suggestion"));
					content.setFeedback(feedback);
					
					String jsonString = mapper.writeValueAsString(content);
					notification.setContent(jsonString);	
					notification.setPriority(NotificationPriority.HIGH);
					this.localDataManager.processNotification(notification);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			}
			
		} else if(measurement.getConstantQuantityMeasureType() == ConstantQuantityMeasureType.LIQUIFIED_PETROLEUM_GAS_MEASUREMENT){
			if(Integer.valueOf(measurement.getMeasuredValue()) > Integer.parseInt(threshold.getProperty("gas.lpg.alert"))) {
				//ALARM
				try {
					//how to get current username ???
					Notification notification = new Notification();
					NotificationContentMsg content = new NotificationContentMsg();
					List<NotificationContentFeedbackMsg> feedback = 
							new ArrayList<NotificationContentFeedbackMsg>();
					
					NotificationContentFeedbackMsg feed1 = new NotificationContentFeedbackMsg();
						feed1.setType("button");
						feed1.setLabel(msg.getProperty("gas.feedback.label"));
						feed1.setUrl("");
					
					feedback.add(feed1);
					
					notification.setTitle(msg.getProperty("gas.notification.title.alarm"));
					
					content.setType(msg.getProperty("gas.content.type"));
					content.setSubtype(msg.getProperty("gas.lpg.content.title.alarm"));
					content.setTitle(msg.getProperty("gas.lpg.content.title.alarm"));
					content.setMotivational(msg.getProperty("gas.lpg.content.motivational"));
					content.setSuggestion(msg.getProperty("gas.lpg.content.suggestion"));
					content.setFeedback(feedback);
						
					String jsonString = mapper.writeValueAsString(content);
					notification.setContent(jsonString);	
					notification.setPriority(NotificationPriority.HIGH);
					this.localDataManager.processNotification(notification);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			}
		}
		return false;
	}

	@Override
	public boolean activitySensingDataEvent(AccelerometerMeasurement accMeasurement) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean appliancePowerSensingDataEvent(AppliancePowerSensing appliancePowerSensing) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean furnitireSensingDataEvent(MattressPressureSensing pressureSensing) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean speakerSensingDataEvent(SpeakerSensing speakerSensing) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean visualSensingDataEvent(VisualSensing visualSensing) {
		// TODO Auto-generated method stub
		return false;
	}

	/**
	 * returns true when an alert is raised
	 * returns false when we have a warning or nothing
	 */
	@Override
	public boolean vitalsSensingDataEvent(VitalsSensing vitalsSensing) {
		Properties msg = new Properties();
		Integer osWarning, osAlert;
		osWarning = Integer.valueOf(threshold.getProperty("os.warning"));
		osAlert = Integer.valueOf(threshold.getProperty("os.alert"));
		
		//TODO just an example, remove these block (next 4 lines)
		if (systemPreferences != null) {
			logger.info("User prefered language = "+systemPreferences.getPreferredLanguage());
			logger.info("User secondary language = "+systemPreferences.getSecondaryLanguage());
			msg = languagesMap.get(systemPreferences.getPreferredLanguage());
		} else {
			msg = languagesMap.get("en");
		}
		
		VitalsSensingDataQueue vsdq = VitalsSensingDataQueue.getInstance();
		if(new Date().getTime() - lastMeasurement.getTime() > 3600000 ) {
			System.out.println("List reset");
			vsdq.empty();
		}
		//if there are no measurements in the queue
		//fill the queue with the first measurement
		if(vsdq.getQueue().size()<5) {
			vsdq.getQueue().add(vitalsSensing.getOxygenSaturation());
			lastMeasurement = new Date();
			return false;
		} 
		//use the queue to get the median of the last 5 measurements
		if(vsdq.getQueue().size()==5){
			vsdq.getQueue().remove();
			vsdq.getQueue().add(vitalsSensing.getOxygenSaturation());
			
			List<Integer> temp = new ArrayList<Integer>(vsdq.getQueue());
			Collections.sort(temp);
	
			//create the notification
			Notification notification = new Notification();
			
			
			NotificationContentMsg content = new NotificationContentMsg();
			List<NotificationContentFeedbackMsg> feedback = 
					new ArrayList<NotificationContentFeedbackMsg>();
			if(new Date().getTime() - osWarningTimer.getTime() > 180000) {
				System.out.println("warn timer reset");
				osWarningSent = false;
			}
			if(new Date().getTime() - osAlarmTimer.getTime() > 180000) {
				System.out.println("alarm timer reset");
				osAlarmSent = false;
			}
			try {
				//set notification data
				NotificationContentFeedbackMsg feed1 = new NotificationContentFeedbackMsg();
						feed1.setType("button");
						feed1.setLabel(msg.getProperty("os.feedback.label")); // msg.prop
						feed1.setUrl("");
					
					feedback.add(feed1);
					
					content.setType(msg.getProperty("os.content.type"));// msg.prop
					content.setMotivational(msg.getProperty("os.content.motivational"));// msg.prop
					content.setSuggestion(msg.getProperty("os.content.suggestion"));// msg.prop
					content.setFeedback(feedback);
					
					String jsonString = mapper.writeValueAsString(content);
					notification.setContent(jsonString);
					notification.setPriority(NotificationPriority.HIGH);
					
				//check if we are below the warning threshold
				//and process the notification
				if(temp.get(2)< osWarning && temp.get(2) > osAlert) {
					//Send a warning
					//there is no specified warning message in the model 
					//so I am sending an alarm instead
					//TODO change type to warning
					notification.setTitle(msg.getProperty("os.notification.title.warn"));//msg.prop
					content.setSubtype(msg.getProperty("os.content.title.warn"));//msg.prop
					content.setTitle(msg.getProperty("os.content.title.warn"));// msg.prop
					if(!osWarningSent) {
						this.localDataManager.processNotification(notification);
						osWarningSent = true;
						osWarningTimer = new Date();
					}
					return false;
				//check if we are below the alarm threshold 	
				} else if(temp.get(2) < osAlert) {
					// RAISE ALARM
					notification.setTitle(msg.getProperty("os.notification.title.alarm"));//msg.prop
					content.setSubtype(msg.getProperty("os.content.title.alarm"));//msg.prop
					content.setTitle(msg.getProperty("os.content.title.alarm"));//msg.prop
					if(!osAlarmSent) {
						this.localDataManager.processNotification(notification);
						osAlarmSent = true;
						osAlarmTimer = new Date();
					}
					return true;
				}
				
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
		
		
		
		return false;
	}

}
