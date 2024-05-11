#ifdef useMQTT
void mqtt_setup(void);
void mqtt_loop(void);
bool mqtt_publish_tele1(void);
bool mqtt_publish_tele2(void);
bool mqtt_publish_tele3(void);
bool mqtt_publish_tele4(void);
bool mqtt_publish_stat_maxTemp();
bool mqtt_publish_stat_minTemp();
bool mqtt_publish_stat_offsetTemp();
bool mqtt_publish_stat_mode();
bool mqtt_publish_stat_manual();
bool mqtt_publish_stat_manualPWM();
bool mqtt_publish_stat_minimumPWM();
#ifdef useShutdownButton
bool mqtt_publish_shutdown();
#endif
#ifdef useHomeassistantMQTTDiscovery
/* Sets the start of the timer until HA discovery is sent.
   It will be waited WAITAFTERHAISONLINEUNTILDISCOVERYWILLBESENT ms before the discovery is sent.
   0: discovery will not be sent
   >0: discovery will be sent as soon as "WAITAFTERHAISONLINEUNTILDISCOVERYWILLBESENT" ms are over
*/
extern unsigned long timerStartForHAdiscovery;
bool mqtt_publish_hass_discovery();
#endif
#endif