// =================================
// ========== FILE INCLUDE =========
// =================================

#include "oled.h"        // Header định nghĩa hàm giao tiếp OLED
#include "i2c.h"         // Giao tiếp I2C để gửi lệnh/dữ liệu cho OLED
#include <string.h>      // Dùng strlen để tính độ dài chuỗi
#include <stdio.h>       // Dùng sprintf để format văn bản
#include "system.h"      // Hàm Delay_ms (trì hoãn sau khi khởi tạo)


// =======================================
// ========== FUNCTION DEFINITIONS =======
// =======================================

/**
 * @brief Bảng font 5x8: mỗi ký tự được biểu diễn bằng 5 byte, mỗi byte là 1 cột pixel (8 dòng).
 *        Dùng để hiển thị ký tự cơ bản: chữ cái, số, khoảng trắng.
 *        Ví dụ: chữ 'A' ứng với 5 cột dữ liệu hiển thị.
 */
const uint8_t font5x8[][5] = {
    // A–Z (0–25)
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x7F,0x20,0x18,0x20,0x7F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x03,0x04,0x78,0x04,0x03}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z

    // a–z (26–51)
    {0x20,0x54,0x54,0x54,0x78}, // a
    {0x7F,0x48,0x44,0x44,0x38}, // b
    {0x38,0x44,0x44,0x44,0x20}, // c
    {0x38,0x44,0x44,0x48,0x7F}, // d
    {0x38,0x54,0x54,0x54,0x18}, // e
    {0x08,0x7E,0x09,0x01,0x02}, // f
    {0x0C,0x52,0x52,0x52,0x3E}, // g
    {0x7F,0x08,0x04,0x04,0x78}, // h
    {0x00,0x44,0x7D,0x40,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00}, // j
    {0x7F,0x10,0x28,0x44,0x00}, // k
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78}, // m
    {0x7C,0x08,0x04,0x04,0x78}, // n
    {0x38,0x44,0x44,0x44,0x38}, // o
    {0x7C,0x14,0x14,0x14,0x08}, // p
    {0x08,0x14,0x14,0x18,0x7C}, // q
    {0x7C,0x08,0x04,0x04,0x08}, // r
    {0x48,0x54,0x54,0x54,0x20}, // s
    {0x04,0x3F,0x44,0x40,0x20}, // t
    {0x3C,0x40,0x40,0x20,0x7C}, // u
    {0x1C,0x20,0x40,0x20,0x1C}, // v
    {0x3C,0x40,0x30,0x40,0x3C}, // w
    {0x44,0x28,0x10,0x28,0x44}, // x
    {0x0C,0x50,0x50,0x50,0x3C}, // y
    {0x44,0x64,0x54,0x4C,0x44}, // z

    // 0–9 (52–61)
    {0x3E,0x45,0x49,0x51,0x3E}, // 0
    {0x00,0x41,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9

    // Space (62)
    {0x00,0x00,0x00,0x00,0x00}
};


/**
 * @brief Gửi 1 lệnh điều khiển (command) tới OLED qua I2C
 * @param cmd Lệnh cần gửi (ví dụ: bật/tắt, set địa chỉ, ...)
 * @return 1 nếu thành công, 0 nếu lỗi
 */
uint8_t SSD1306_Command(uint8_t cmd) {
    // Gửi 1 byte command: control byte = 0x00 (lệnh)
    return I2C_WriteByte(0x3C, 0x00, cmd); // 0x3C là địa chỉ I2C của SSD1306
}


/**
 * @brief Gửi 1 byte dữ liệu (data) tới OLED – dùng để hiển thị pixel
 * @param data Byte dữ liệu hình ảnh (1 byte điều khiển 8 pixel dọc)
 * @return 1 nếu thành công, 0 nếu lỗi
 */
uint8_t SSD1306_Data(uint8_t data) {
    // Gửi 1 byte data: control byte = 0x40 (dữ liệu hiển thị)
    return I2C_WriteByte(0x3C, 0x40, data);
}


/**
 * @brief Khởi tạo OLED theo chuỗi lệnh chuẩn của SSD1306
 * @return 1 nếu khởi tạo thành công, 0 nếu lỗi
 */
uint8_t SSD1306_Init(void) {
    Delay_ms(100);  // Chờ nguồn ổn định trước khi bắt đầu

    // Dãy lệnh khởi tạo SSD1306: cấu hình chế độ, tắt/mở, phân cực,...
    const uint8_t init_seq[] = {
        0xAE,       // Display OFF
        0xD5, 0x80, // Set display clock divide ratio/oscillator frequency
        0xA8, 0x3F, // Set multiplex ratio (1/64)
        0xD3, 0x00, // Set display offset = 0
        0x40,       // Set start line = 0
        0x8D, 0x14, // Enable charge pump
        0x20, 0x00, // Set memory addressing mode: Horizontal
        0xA1,       // Set segment remap (flip horizontal)
        0xC8,       // COM output scan direction: remap (flip vertical)
        0xDA, 0x12, // COM pins configuration
        0x81, 0xCF, // Contrast control
        0xD9, 0xF1, // Pre-charge period
        0xDB, 0x40, // VCOMH deselect level
        0xA4,       // Entire display ON from RAM
        0xA6,       // Normal display (không đảo màu)
        0xAF        // Display ON
    };

    // Gửi từng lệnh trong chuỗi khởi tạo
    for (int i = 0; i < sizeof(init_seq); i++) {
        if (!SSD1306_Command(init_seq[i])) return 0; // Nếu có lệnh lỗi => return fail
    }
    return 1;
}


/**
 * @brief Đặt con trỏ pixel tại vị trí (col, page) để chuẩn bị vẽ dữ liệu
 * @param col Cột (0–127)
 * @param page Dòng theo trang (0–7), mỗi trang là 8 pixel theo chiều dọc
 */
void SSD1306_SetCursor(uint8_t col, uint8_t page) {
    SSD1306_Command(0xB0 + page);             // Lệnh chọn page (dòng)
    SSD1306_Command(0x00 + (col & 0x0F));     // Cột - 4 bit thấp
    SSD1306_Command(0x10 + ((col >> 4) & 0x0F)); // Cột - 4 bit cao
}


/**
 * @brief Xóa toàn bộ màn hình OLED (vẽ toàn bộ bằng màu đen)
 */
void SSD1306_Clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        SSD1306_SetCursor(0, page); // Di chuyển tới đầu mỗi dòng
        for (uint8_t col = 0; col < 128; col++) {
            SSD1306_Data(0x00);     // Gửi giá trị 0 để tắt tất cả pixel
        }
    }
}


/**
 * @brief In 1 ký tự ra màn hình OLED tại vị trí hiện tại
 * @param ch Ký tự ASCII cần hiển thị ('A'–'Z', 'a'–'z', '0'–'9', ...)
 */
void SSD1306_PrintChar(char ch) {
    const uint8_t* chr;

    // Chọn dữ liệu font tương ứng với ký tự
    if (ch >= 'A' && ch <= 'Z')       chr = font5x8[ch - 'A'];
    else if (ch >= 'a' && ch <= 'z')  chr = font5x8[ch - 'a' + 26];
    else if (ch >= '0' && ch <= '9')  chr = font5x8[ch - '0' + 52];
    else                              chr = font5x8[62];  // space

    // Gửi 5 byte bitmap của ký tự
    for (int i = 0; i < 5; i++) SSD1306_Data(chr[i]);

    // Gửi thêm 1 cột trắng (khoảng cách giữa các ký tự)
    SSD1306_Data(0x00);
}


/**
 * @brief In một chuỗi ký tự canh giữa theo chiều ngang tại 1 dòng (page)
 * @param page Dòng cần in (0–7)
 * @param str Chuỗi ký tự cần hiển thị
 */
void SSD1306_PrintTextCentered(uint8_t page, const char* str) {
    uint8_t len = strlen(str);                 // Tính độ dài chuỗi
    uint8_t col = (128 - len * 6) / 2;         // Tính vị trí canh giữa (5 byte font + 1 byte khoảng trắng)
    SSD1306_SetCursor(col, page);              // Di chuyển con trỏ tới vị trí canh giữa
    while (*str) SSD1306_PrintChar(*str++);    // In từng ký tự
}


/**
 * @brief Hiển thị trạng thái thiết bị (mode hiện tại và thời gian)
 * @param current_mode Chế độ hiện tại (ví dụ: 1–3)
 * @param seconds_left Số giây còn lại (nếu = 0 thì hiển thị READY)
 */
void SSD1306_DisplayStatus(uint8_t current_mode, uint8_t seconds_left) {
    char buffer[32];                        // Chuỗi tạm để format thông tin

    SSD1306_Clear();                        // Xóa toàn bộ màn hình

    SSD1306_PrintTextCentered(1, "DEVICE STATUS"); // In tiêu đề

    sprintf(buffer, "MODE %d", current_mode);      // Ghi mode hiện tại
    SSD1306_PrintTextCentered(3, buffer);

    if (seconds_left > 0)
        sprintf(buffer, "TIME %ds", seconds_left); // Ghi thời gian đếm ngược
    else
        sprintf(buffer, "READY");                  // Nếu hết giờ thì báo sẵn sàng

    SSD1306_PrintTextCentered(5, buffer);
}


// =======================================
// ============= END FILE ================
// =======================================
