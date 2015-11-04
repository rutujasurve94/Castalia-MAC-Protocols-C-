
enum LanternLMacCommandDef {
SET_DUTY_CYCLE = 1;
SET_LISTEN_INTERVAL = 2;
SET_BEACON_INTERVAL_FRACTION = 3;
SET_PROB_TX = 4;
SET_NUM_TX = 5;
SET_RANDOM_TX_OFFSET = 6;
SET_RETX_INTERVAL = 7;
SET_BACKOFF_TYPE = 8;
SET_BACKOFF_BASE_VALUE = 9;
}
message LanternMacControlCommand {
int lanternMacCommandKind enum (LanternMacCommandDef);
double parameter = 0;
}