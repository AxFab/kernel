#ifndef KERNEL_ASYNC_H__
#define KERNEL_ASYNC_H__

struct kEvent 
{
  char size_;
  char type_;

  union {

    struct {
      char rel_x_;
      char rel_y_;
    } mouse_motion_;

    struct {
      short button_;
    } mouse_button_;

    struct {
      short key_;
    } keyboard_;

  };
};


#endif /* KERNEL_ASYNC_H__ */
