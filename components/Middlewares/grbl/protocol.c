#include "protocol.h"
#include "stdint.h"
#include "Serial.h"
#include "string.h"

// Define line flags. Includes comment type tracking and line overflow detection.
#ifndef bit(n)
#define bit(n) (1 << (n))
#endif

#define LINE_FLAG_OVERFLOW bit(0)            // 第1位bit，表示是否溢出
#define LINE_FLAG_COMMENT_PARENTHESES bit(1) // 第2位bit，表示是否括号注释
#define LINE_FLAG_COMMENT_SEMICOLON bit(2)   // 第3位bit，表示是否分号注释

static char line[LINE_BUFFER_SIZE]; // Line to be executed. Zero-terminated.

/*
  GRBL PRIMARY LOOP:
*/
void protocol_main_loop()
{

    // ---------------------------------------------------------------------------------
    // Primary loop! Upon a system abort, this exits back to main() to reset the system.
    // This is also where Grbl idles while waiting for something to do.
    // ---------------------------------------------------------------------------------

    uint8_t line_flags = 0;   // 读取状态标记
    uint8_t char_counter = 0; // 改行读取的有效字符的计数器（不超过LINE_BUFFER_SIZE）
    uint8_t c;
    for (;;)
    {

        // Process one line of incoming serial data, as the data becomes available. Performs an
        // initial filtering by removing spaces and comments and capitalizing all letters.
        // 逐行处理串口接受到的数据，如果有数据到来，进行初步过滤，去除空格和注释，并将所有字母转为大写。
        while ((c = serial_read()) != SERIAL_NO_DATA) // 当有数据时
        {
            if ((c == '\n') || (c == '\r')) // 读到末尾
            {                               // End of line reached

                printf("protocol_execute_realtime()实时命令运行开始\n");
                printf("检查sys.abort是否为真,来判断是否要回到main函数重启\n");
                // protocol_execute_realtime(); // Runtime command check point.
                // if (sys.abort)
                // {
                //     return;
                // } // Bail to calling function upon system abort

                line[char_counter] = 0; // Set string termination character.（末尾标记）

                // Direct and execute one line of formatted input, and report status of execution.（执行）
                if (line_flags & LINE_FLAG_OVERFLOW) // 读取的字符溢出
                {
                    // Report line overflow error.（报错）
                    printf("the line is overflow\n");
                    // report_status_message(STATUS_OVERFLOW);
                }
                else if (line[0] == 0) // 未读到字符
                {
                    // Empty or comment line. For syncing purposes.
                    printf("the line is empty");
                    // report_status_message(STATUS_OK);
                }
                else if (line[0] == '$') // 系统命令--》system函数调用
                {
                    // Grbl '$' system command
                    printf("the line is a system command,begin to execute\n");
                    // report_status_message(system_execute_line(line));
                }
                // else if (sys.state & (STATE_ALARM | STATE_JOG)) // 系统处在ALARM或者JOG状态，不执行命令
                // {
                //     // Everything else is gcode. Block if in alarm or jog mode.
                //     printf("the system is in alarm or jog mode,block the gcode command\n");
                //     // report_status_message(STATUS_SYSTEM_GC_LOCK);
                // }
                else // 正常的gcode命令
                {
                    // Parse and execute g-code block.
                    printf("the line is a gcode command,begin to execute\n");
                    char temp_line[LINE_BUFFER_SIZE];
                    memcpy(temp_line, line, char_counter);
                    temp_line[char_counter] = '\0';
                    printf("the line is %s\n", temp_line);

                    // report_status_message(gc_execute_line(line));
                }

                // Reset tracking data for next line.（为下一行读取做准备）
                line_flags = 0;
                char_counter = 0;
            }
            else // 读到中间（没有读到末尾）--》读取字母，数字，遇到注释和异常符号自动清除
            {

                if (line_flags) // 存在状态标记
                {
                    // Throw away all (except EOL) comment characters and overflow characters.
                    if (c == ')') // 括号注释结束
                    {
                        // End of '()' comment. Resume line allowed.
                        if (line_flags & LINE_FLAG_COMMENT_PARENTHESES) // 如果flag处在括号标记状态，则清除
                        {
                            line_flags &= ~(LINE_FLAG_COMMENT_PARENTHESES);
                        }
                    }
                }
                else // 不存在状态标记（正常读取）
                {
                    if (c <= ' ')
                    {
                        // Throw away whitepace and control characters
                    }
                    else if (c == '/')
                    {
                        // Block delete NOT SUPPORTED. Ignore character.
                        // NOTE: If supported, would simply need to check the system if block delete is enabled.
                    }
                    else if (c == '(') // 开始括号注释
                    {
                        // Enable comments flag and ignore all characters until ')' or EOL.
                        // NOTE: This doesn't follow the NIST definition exactly, but is good enough for now.
                        // In the future, we could simply remove the items within the comments, but retain the
                        // comment control characters, so that the g-code parser can error-check it.
                        line_flags |= LINE_FLAG_COMMENT_PARENTHESES;
                    }
                    else if (c == ';') // 开始分号注释
                    {
                        // NOTE: ';' comment to EOL is a LinuxCNC definition. Not NIST.
                        line_flags |= LINE_FLAG_COMMENT_SEMICOLON;
                        // TODO: Install '%' feature
                        // } else if (c == '%') {
                        // Program start-end percent sign NOT SUPPORTED.
                        // NOTE: This maybe installed to tell Grbl when a program is running vs manual input,
                        // where, during a program, the system auto-cycle start will continue to execute
                        // everything until the next '%' sign. This will help fix resuming issues with certain
                        // functions that empty the planner buffer to execute its task on-time.
                    }
                    else if (char_counter >= (LINE_BUFFER_SIZE - 1)) // 超过最大长度
                    {
                        // Detect line buffer overflow and set flag.
                        line_flags |= LINE_FLAG_OVERFLOW;
                    }
                    else if (c >= 'a' && c <= 'z') // 为正常的小写字母
                    {                              // Upcase lowercase
                        line[char_counter++] = c - 'a' + 'A';
                    }
                    else // 为正常的大写字母或者数字
                    {
                        line[char_counter++] = c;
                    }
                }
            }
        }

        // If there are no more characters in the serial read buffer to be processed and executed,
        // this indicates that g-code streaming has either filled the planner buffer or has
        // completed. In either case, auto-cycle start, if enabled, any queued moves.
        // protocol_auto_cycle_start();
        printf("protocol_auto_cycle_start()自动循环\n");

        // protocol_execute_realtime(); // Runtime command check point.
        printf("protocol_execute_realtime()实时命令运行开始\n");
        printf("检查sys.abort是否为真,来判断是否要回到main函数重启\n");
        // if (sys.abort)
        // {
        //     return;
        // } // Bail to main() program loop to reset system.
        printf("准备return,后续整合时，这里的return可以考虑删除掉\n");
        return;
    }

    return; /* Never reached */
}