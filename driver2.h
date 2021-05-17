
int init_module(void);
void cleanup_module(void);
static ssize_t device_read(struct file *, char *, size_t,loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);