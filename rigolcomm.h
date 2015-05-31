#ifndef RIGOLCOMM_H
#define RIGOLCOMM_H


class RigolComm
{
protected:
    int fd;
    char *_buffer;
    int get_data_size(int rawsize);

public:
    char *buffer;
    unsigned data_size;

    RigolComm();
    ~RigolComm();
    int open(const char *device);
    int close(void);
    bool connected(void) { return fd>=0 && _buffer; }
    int send(const char *command);
    int unlock(void);
    int recv(void);
    float toFloat(void);
    float cmdFloat(const char *cmd);
    float cmdFloatlt(const char *cmd);
    int command(const char *cmd);
};

#endif // RIGOLCOMM_H
