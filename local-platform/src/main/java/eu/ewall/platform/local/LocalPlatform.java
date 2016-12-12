/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.local;

import java.io.IOException;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import eu.ewall.platform.local.datamanager.LocalDataManager;
import eu.ewall.platform.local.datamanager.couchdb.CouchDbPullRequestManager;
import eu.ewall.platform.local.reasoners.MainReasoner;
import eu.ewall.platform.remoteproxy.RemoteProxyImpl;
import eu.ewall.platform.remoteproxy.api.RemoteProxyProvider;

/**
 * The Class LocalPlatform.
 * 
 * @author emirmos
 */
public class LocalPlatform {
	/** The logger. */
	Logger logger = LoggerFactory.getLogger(LocalPlatform.class);
	
	private final int RETRY_IN_MINUTES = 5;
	private final int NUMBER_OF_RETRIES = 12;

	/** The remote proxy provider. */
	private RemoteProxyProvider remoteProxyProvider;

	/** The local data manager. */
	private LocalDataManager localDataManager;

	/** The reasoner. */
	// private ExampleReasoner reasoner;

	private MainReasoner reasoner;

	/**
	 * The main method.
	 *
	 * @param args
	 *            the arguments
	 * @throws IOException
	 *             Signals that an I/O exception has occurred.
	 */
	public static void main(String[] args) throws IOException {
		boolean resume = false;
		for (String paramater : args) {
			if ("-resume".equals(paramater))
				resume = true;
		}
		LocalPlatform localPlatfrom = new LocalPlatform(resume);
		localPlatfrom.startup();
	}

	/**
	 * Instantiates a new local platform.
	 *
	 * @param resume
	 *            the resume
	 */
	public LocalPlatform(boolean resume) {
		super();
		remoteProxyProvider = new RemoteProxyImpl();
		remoteProxyProvider.setRemoteProxyDBProvider(CouchDbPullRequestManager.getInstance());
		localDataManager = new LocalDataManager(resume);
		reasoner = new MainReasoner(localDataManager);
	}

	/**
	 * Startup.
	 *
	 * @return true, if successful
	 */
	public boolean startup() {
		// we need to add hook, so when Ctr+C is pressed, local platform will
		// stop gracefully
		Runtime.getRuntime().addShutdownHook(new Thread() {
			@Override
			public void run() {

				shutdown();

				try {
					// wait for local data manager to stop
					Thread.sleep(10000);
				} catch (InterruptedException e) {
					logger.error("Interrup exception during shutdown", e);
				}

				logger.info("Local platform is stopped.");
			}

		});

		logger.info("Starting local platform components...");
		
		boolean sucess = remoteProxyProvider.startup();
		int iteration = 0;
		
		while (!sucess && iteration < NUMBER_OF_RETRIES) {
			logger.info("Error in starting remote proxy. Retrying in {} minutes ({} more time(s)).", RETRY_IN_MINUTES, NUMBER_OF_RETRIES-iteration);
			try {
				TimeUnit.MINUTES.sleep(RETRY_IN_MINUTES);
			} catch (InterruptedException e) {
				logger.error("Interrupt exception during pause in starting remote proxy.", e);
			}
			iteration++;
			sucess = remoteProxyProvider.startup();
		}
		
		if (!sucess) {
			logger.error("Error in starting remote proxy. Shutting down local platform...");
			shutdown();
			return false;
		}
		
		logger.info("Remote proxy started.");

		if (!localDataManager.startup(remoteProxyProvider.getRPCloudClient())) {
			logger.error("Error in starting local data manager.");
			shutdown();
			return false;
		}
		logger.info("Local data manager started.");

		if (!reasoner.startup()) {
			logger.error("Error in starting local reasoner.");
			shutdown();
			return false;

		}
		logger.info("Local reasoner started.");
		logger.info("Local platform components started successfully.");
		return true;

	}

	/**
	 * Shutdown.
	 */
	public void shutdown() {
		logger.info("Stopping local platform components...");

		if (!reasoner.shutdown()) {
			logger.warn("Local reasoner did not stop.");
		} else
			logger.info("Local reasoner stopped.");

		if (!localDataManager.shutdown()) {
			logger.warn("Local data manager did not stop.");
		} else
			logger.info("Local data manager stopped.");

		if (!remoteProxyProvider.shutdown()) {
			logger.warn("Remote proxy did not stop.");
		} else
			logger.info("Remote proxy stopped.");

	}
}
