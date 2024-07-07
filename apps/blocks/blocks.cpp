#include <array>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include "../syscall.h"

using namespace std;

const int kNumBlocksX = 10, kNumBlocksY = 5;
const int kBlockWidth = 20, kBlockHeight = 10;
const int kBarWidth = 30, kBarHeight = 5, kBallRadius = 5;
const int kGapWidth = 30, kGapHeight = 30, kGapBar = 80, kBarFloat = 10;

const int kCanvanWidth = kNumBlocksY*kBlockWidth + 2*kGapWidth;
const int kCanvanHeight = kGapHeight + kNumBlocksY*kBlockHeight + kGapBar + kBarHeight + kBarFloat;
const int kBarY = kCanvanHeight - kBarFloat - kBarHeight;

const int kFrameRate = 60;
const int kBarSpeed = kCanvanWidth /2 ;
const int kBallSpeed = kBarSpeed;

array <bitset<kNumBlocksX>, kNumBlocksY> blocks;

void DrawBlocks(uint64_t layer_id){
    for (int by = 0; by < kNumBlocksY; ++by){
        const int y = 24 + kGapHeight + by*kBlockHeight;
        const uint32_t color = 0xff << (by%3) * 8;

        for (int bx = 0; bx < kNumBlocksX; ++bx){
            if (blocks[by][bx]){
                const int x = 4+kGapWidth + bx*kBlockWidth;
                const uint32_t c = color | (0xff << ((bx+by)%3)*8);
                SyscallWinFillRectangle(layer_id, x, y, kBlockWidth, kBlockHeight, c);
            }
        }
    }
}

void DrawBar(uint64_t layer_id, int bar_x){
    SyscallWinFillRectangle(layer_id, 4+bar_x, 24+kBarY, kBarWidth, kBarHeight, 0xffffff);
}

void DrawBall(uint64_t layer_id, int x, int y){
    SyscallWinFillRectangle(layer_id, 4+x-kBallRadius, 24+y-kBallRadius, 2*kBallRadius,2*kBallRadius, 0x007f00);
    SyscallWinFillRectangle(layer_id, 4+x-kBallRadius/2, 24+y-kBallRadius/2, kBallRadius, kBallRadius, 0x00ff00);
}

template <class T>
T LimitRange(const T& x, const T& min, const T& max){
    if (x<min){
        return min;
    } else if (x>max){
        return max;
    }
    return x;
}

extern "C" void main(int argc, char** argv){
    auto [layer_id, err_openwin] = SyscallOpenWindow(kCanvanWidth+8, kCanvanHeight+28, 10, 10, "blocks");
    if (err_openwin){
        exit(err_openwin);
    }

    for (int y=0; y<kNumBlocksY; y++){
        blocks[y].set();
    }

    const int kBallX = kCanvanWidth/2 - kBallRadius - 20;
    const int kBallY = kCanvanHeight - kBarFloat - kBarHeight - kBallRadius - 20;

    int bar_x = kCanvanWidth/2 - kBarWidth/2;
    int ball_x = kBallX, ball_y = kBallY;
    int move_dir = 0;
    int ball_dir = 0;
    int ball_dx = 0, ball_dy = 0;

    for (;;){
        SyscallWinFillRectangle(layer_id | LAYER_NO_REDRAW, 4, 24, kCanvanWidth, kCanvanHeight, 0);
        DrawBlocks(layer_id | LAYER_NO_REDRAW);
        DrawBar(layer_id | LAYER_NO_REDRAW, bar_x);
        if (ball_y >= 0){
            DrawBall(layer_id | LAYER_NO_REDRAW, ball_x, ball_y);
        }
        SyscallWinRedraw(layer_id);

        static unsigned long prev_timeout = 0;
        if (prev_timeout == 0){
            const auto timeout = SyscallCreateTimer(TIMER_ONESHOT_REL, 1, 1000/kFrameRate);
            prev_timeout = timeout.value;
        } else {
            prev_timeout += 1000/ kFrameRate;
            SyscallCreateTimer(TIMER_ONESHOT_ABS, 1, prev_timeout);
        }

        AppEvent events[1];
        for (;;){
            SyscallReadEvent(events,1);
            if (events[0].type == AppEvent::kTimerTimeout){
                break;
            } else if (events[0].type == AppEvent::kQuit){
                goto fin;
            } else if (events[0].type == AppEvent::kKeyPush){
                if (!events[0].arg.keypush.press){
                    move_dir = 0;
                } else {
                    const auto keycode = events[0].arg.keypush.keycode;
                    if (keycode == 79){
                        move_dir = 1;
                    } else if (keycode == 80){
                        move_dir = -1;
                    } else if (keycode == 44){
                        if (ball_dir == 0 && ball_y < 0){
                            ball_x = kBallX; ball_y = kBallY;
                        } else if (ball_dir == 0){
                            ball_dir = 45;
                        }
                    }
                    if (bar_x == 0 && move_dir<0){
                        move_dir = 0;
                    } else if (bar_x + kBarWidth == kCanvanWidth-1 && move_dir>0){
                        move_dir = 0;
                    }
                }
            }
        }

        bar_x += move_dir*kBarSpeed / kFrameRate;
        bar_x = LimitRange(bar_x, 0, kCanvanWidth-kBarWidth-1);

        if (ball_dir == 0){
            continue;
        }

        int ball_x_ = ball_x + ball_dx, ball_y_ = ball_y + ball_dy;
        if ((ball_dx<0 && ball_x_<kBallRadius) || (ball_dx>0 && kCanvanWidth-kBallRadius <= ball_x_)){
            //壁
            ball_dir = 180-ball_dir;
        }
        if (ball_dy<0 && ball_y_<kBallRadius){
            ball_dir = -ball_dir;
        } else if (bar_x <= ball_x_ && ball_x_ < bar_x + kBarWidth && ball_dy>0 && kBarY-kBallRadius<=ball_y_) {
            ball_dir = -ball_dir;
        }else if (ball_dy>0 && kCanvanHeight-kBallRadius<=ball_y_){
            ball_dir = 0;
            ball_y = -1;
            continue;
        }

        do {
            if (ball_x_ < kGapWidth || kCanvanWidth-kGapWidth <= ball_x_ || ball_y_<kGapHeight || kGapHeight+kNumBlocksY*kBlockHeight<=ball_y_){
                break;
            }

            const int index_x = (ball_x_ - kGapWidth) / kBlockWidth;
            const int index_y = (ball_y_ - kGapHeight) / kBlockHeight;
            if (!blocks[index_y].test(index_x)){
                break;
            }

            blocks[index_y].reset(index_x);

            const int block_left = kGapWidth + index_x*kBlockWidth;
            const int block_right = kGapWidth + (index_x+1)*kBlockWidth;
            const int block_top = kGapHeight + (index_y)*kBlockHeight;
            const int block_bottom = kGapHeight + (index_y+1)*kBlockHeight;

            if ((ball_x < block_left && block_left <= ball_x_) || (block_right< ball_x && ball_x_<=block_right)){
                ball_dir = 180-ball_dir;
            }
            if ((ball_y < block_top && block_top <= ball_y_) || (block_bottom< ball_y && ball_y_<=block_bottom)){
                ball_dir = -ball_dir;
            }
        } while (false);

        ball_dx = round(kBallSpeed*cos(M_PI * ball_dir/180)/kFrameRate);
        ball_dy = round(kBallSpeed*sin(M_PI * ball_dir/180)/kFrameRate);
        ball_x += ball_dx;
        ball_y += ball_dy;
    }

    fin:
        SyscallCloseWindow(layer_id);
        exit(0);
}