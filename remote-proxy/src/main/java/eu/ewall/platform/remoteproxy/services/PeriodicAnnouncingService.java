/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.services;

import static java.util.concurrent.TimeUnit.*;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import eu.ewall.platform.remoteproxy.RemoteProxyImpl;

/**
 * The Class PeriodicAnnouncingService.
 */
public class PeriodicAnnouncingService {

	/** The log. */
	private static final Logger LOG = LoggerFactory
			.getLogger(PeriodicAnnouncingService.class);

	/** The scheduler. */
	private final ScheduledExecutorService scheduler = Executors
			.newSingleThreadScheduledExecutor();

	/** The remote proxy impl. */
	private RemoteProxyImpl remoteProxyImpl;

	/** The Constant ANNOUNCING_INTERVAL. */
	private static final int ANNOUNCING_INTERVAL = 4 * 60 * 1000; // 4 min

	/**
	 * Instantiates a new periodic announcing service.
	 *
	 * @param remoteProxyImpl
	 *            the remote proxy impl
	 */
	public PeriodicAnnouncingService(RemoteProxyImpl remoteProxyImpl) {
		this.remoteProxyImpl = remoteProxyImpl;
	}

	/**
	 * Start announcing.
	 */
	public void startAnnouncing() {
		LOG.debug("Starting periodic announcing thread...");
		scheduler.scheduleAtFixedRate(announce, ANNOUNCING_INTERVAL,
				ANNOUNCING_INTERVAL, MILLISECONDS);
	}

	/**
	 * Stop announcing.
	 */
	public void stopAnnouncing() {
		LOG.debug("Stopping periodic announcing thread...");
		scheduler.shutdownNow();
	}

	/** The announce. */
	final Runnable announce = new Runnable() {
		@Override
		public void run() {

			try {
				remoteProxyImpl.doRegister();

			} catch (Exception e) {
				LOG.error("Error while periodically announcing to cloud gateway.", e);
			}
		}
	};

}