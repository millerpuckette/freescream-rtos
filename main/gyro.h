#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

int gyro_init(void);
void gyro_geteulerangles(float *rollp, float *pitchp, float *yawp);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif
