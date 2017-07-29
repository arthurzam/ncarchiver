#ifndef KEYS_H
#define KEYS_H

#define KEY_CTRL(c)     ((c)&0x1f)

#define KEY_SPACE       ' '
#define KEY_DELETE      '\177'	/* Delete key */
#define KEY_TAB         '\t'	/* Tab key.   */
#define KEY_ESC         '\033'	/* Escape Key.*/
#define KEY_RETURN      '\012'	/* Return key */
#define KEY_F1          KEY_F(1)
#define KEY_F2          KEY_F(2)
#define KEY_F3          KEY_F(3)
#define KEY_F4          KEY_F(4)
#define KEY_F5          KEY_F(5)
#define KEY_F6          KEY_F(6)
#define KEY_F7          KEY_F(7)
#define KEY_F8          KEY_F(8)
#define KEY_F9          KEY_F(9)
#define KEY_F10         KEY_F(10)
#define KEY_F11         KEY_F(11)
#define KEY_F12         KEY_F(12)

#define KEY_ERROR       ((chtype)ERR)

#endif // KEYS_H
