#ifndef _SERIAL_H
#define _SERIAL_H

int  serial_init(const char *devStr);
int  serial_close(void);
void send_scope_on(void);
void send_scope_off(void);
void send_scope_wave_output_on(void);
void send_scope_wave_output_off(void);
//void get_scope_waveform_data(void);

#endif /* _SERIAL_H */
