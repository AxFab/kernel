#include <kernel/streams.h>
#include <kernel/memory.h>
#include <kernel/info.h>
#include <kernel/scheduler.h>
#include <kernel/inodes.h>
#include <kernel/uparams.h>
#include <stdio.h>



// // ---------------------------------------------------------------------------
// /** Write a  */
// ssize_t kstm_write_tty (kNTty_t* tty, void* buf, size_t length)
// {
//   int pen;
//   ssize_t count = kstm_write_pipe(tty->output_, buf, length);

//   for (;;) {

//     if (tty->row_ > tty->maxRow_) {
//       kstm_scroll_tty (tty, tty->row_ - tty->maxRow_);
//     }

//     kLine_t line = *tty->last_;
//     pen = paintLine (tty, &line, tty->row_);

//     if (tty->newInput_ && !tty->isInput_) tty->reEcho_ = 1;
//     if (pen == 0)
//       break;

//     kLine_t* newLine =  KALLOC(kLine_t);
//     newLine->offset_ = pen;
//     newLine->txColor_ = line.txColor_;
//     newLine->bgColor_ = line.bgColor_;
//     newLine->flags_ = line.flags_;
//     tty->last_->next_ = newLine;
//     newLine->prev_ = tty->last_;
//     tty->last_ = newLine;

//     //FIXME, delete from first until offset > pen + tty->maxCol_;

//     tty->row_++;
//   }

//   // save_bmp (tty->width_, tty->height_, tty->pixels_);
//   return count;
// }


// // ---------------------------------------------------------------------------
// /** Read a  */
// ssize_t kstm_read_tty  (kNTty_t* tty, void* buf, size_t length)
// {
//   // Should read until '\n'
//   ssize_t count = kstm_read_pipe_line(tty->input_, buf, length);
//   if (count <= 0) {
//     kprintf("ERROR in read_tty\n");
//     return count;
//   }

//   ((char*)buf)[count] = '\0';
//   if (tty->reEcho_) {
//     tty->reEcho_ = 0;
//     if (kstm_available_data_pipe(tty->input_) == 0)
//       tty->newInput_ = 0;
//     length = strlen(buf);
//     tty->isInput_ = 1;
//     kstm_write_tty (tty, buf, length);
//     tty->isInput_ = 0;
//   }

//   return count;
// }


// void kstm_key_tty (kNTty_t* tty, int ch)
// {
//   if (ch == 'U') {

//   } else if (ch == 'D') {

//   } else if (ch > 0 && ch < 0x80) {
//     tty->isInput_ = 1;
//     kstm_write_tty (tty, &ch, 1);
//     kstm_write_pipe(tty->input_, &ch, 1);
//     tty->isInput_ = 0;
//     tty->newInput_ = 1;
//   }
// }



// void kstm_keys_tty (kNTty_t* tty, void* buf, size_t length)
// {
//   while (length > 0) {
//     int ch = *((char*)buf);
//     if (ch > 0 && ch < 0x80) {
//       kstm_key_tty(tty, ch);
//       length--;
//       buf++;
//     } else {
//       printf ("ERROR");

//     }
//   }
// }


// static int fontW = 6;
// static int fontH = 9;

// kNTty_t* kstm_create_tty (int width, int height)
// {
//   kNTty_t* tty = KALLOC(kNTty_t);
//   // tty->input_ = (kPipe_t*)calloc(sizeof(kPipe_t), 1);
//   // tty->output_ = (kPipe_t*)calloc(sizeof(kPipe_t), 1);
//   // tty->input_->length_ = 1024 * 4;
//   // tty->input_->buffer_ = calloc(1024, 4);
//   // tty->output_->length_ = 1024 * 8;
//   // tty->output_->buffer_ = calloc(1024, 8);
//   tty->width_ = width;
//   tty->height_ = height;
//   tty->txColor_ = 0xffa6a6a6;
//   tty->bgColor_ = 0xff323232;
//   tty->pixels_ = (uint32_t*)kalloc(width * 4 * height);
//   tty->first_ = (kLine_t*)kalloc(sizeof(kLine_t));
//   tty->first_->txColor_ = 0xffa6a6a6;
//   tty->first_->bgColor_ = 0xff323232;
//   tty->last_ = tty->first_;
//   tty->top_ = tty->first_;
//   tty->row_ = 1;
//   tty->maxRow_ = (height-1) / (fontH + 1);
//   tty->maxCol_ = (width-2) / fontW;
//   clearScreen(tty);
//   return tty;
// }

// void kstm_destroy_tty (kNTty_t* tty)
// {
//   kfree(tty->pixels_); // Not if FILE !
//   while (tty->first_) {
//     kLine_t* l = tty->first_->next_;
//     kfree (tty->first_);
//     tty->first_ = l;
//   }

//   // free(tty->input_->buffer_);
//   // free(tty->output_->buffer_);
//   // free(tty->input_);
//   // free(tty->output_);
//   free(tty);
// }
