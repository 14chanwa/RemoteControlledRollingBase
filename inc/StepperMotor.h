class StepperMotor {
  public:
    StepperMotor(uint8_t _in1, uint8_t _in2, uint8_t _in3, uint8_t _in4);

    /* Move one step */
    void move(void);

    /* Set direction. 
       1 - clock wise
       0 - counter clock wise
    */
    void setDir(uint8_t _dir);
  
  private:
    uint8_t in1;
    uint8_t in2;
    uint8_t in3;
    uint8_t in4;
    uint8_t dir;
    uint8_t seq;
};