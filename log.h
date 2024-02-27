#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <stdint.h>
#include <string>
#include <memory>
#include <list>
#include <stringstream>
#include <fstream>
#include <map>

namespace sylar {

class Logger;
//日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent();

    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const {  return m_elapse; }
    uint32_t getThreadId() const {  return m_threadId;  }
    uint32_t getFiberId() const {  return m_fiberId;  }
    uint64_t getTime() const {   return m_time;  }
    const std::string& getContent() const { return m_content; }

private:
    const char* m_file = nullptr;           //文件名
    int32_t m_line = 0;                     //行号
    uint32_t m_elapse = 0;                  //程序启动开始到现在的毫秒数
    uint32_t m_threadId = 0;                //线程id
    uint32_t m_fiberId = 0;                 //协程id
    uint64_t m_time = 0;                    //时间戳
    std::string m_content;                  
};

//日志级别
class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(LogLevel::Level level); 
};

//日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);

    //%t       %thread_id %m%n
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        FormatItem(const std::string& fmt = "") {};
        virtual ~FormatItem(){}
        virtual void format(std::ostream os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
    void init(); 
private:
     std::string m_pattern;
     std::vector<FormatItem::ptr> m_items;
};

//日志输出地
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr val) { m_formatter = val;  }
    LogFormatter::ptr getFormatter() const { return m_formatter; }
protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

//日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
  
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::level getLevel() const { return m_level;}
    void setLevel(LogLevel::Level val) { m_level = val; }

    const std::string& getName() const { return m_name;  }
private:
    std::string m_name;              //日志名称
    LogLevel::Level m_level;         //日志级别
    std::list<LogAppender::ptr> m_appenders; //Appender 集合
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(LogLevel::Level, LogEvent::ptr event) override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(LogLevel:: level, LogEvent::ptr event) override;
    
    //重新打开文件，文件打开成功返回true
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
} 

const char* LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name) \
        case LogLevel::name: \
            return #name; \
            break;

        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
    }
    return "UNKNOW";
}

class MessageFormatItem : public LogFormatter:: FormatItem {
public:
    void format(std::ostream os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};
   
class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y%m%d %H%M%S") 
        :m_format(format) {

    }
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getTime();
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile());
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :FormatItem(str), m_string(str) {}
    void format(std::ostream os, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

Logger::Logger(const std::string& name) : m_name(name) {

}
void Logger::addAppender(LogAppender::ptr appender) {
    m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (Level >= m_level) {
        for (auto& i : m_appenders) {
            i->log(level, event);
        }
    }
}
void Logger::debug(LogEvent::ptr event) {
    debug(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event) {
    debug(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event) {
    debug(LogLevel::WARN, event));
}
void Logger::error(LogEvent::ptr event) {
    debug(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event) {
    debug(LogLevel::FATAL, event);
}

FileLogAppender::FileLogAppender(const std:string& filename) 
    : m_filename(filename) {
    
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::level, LogEvent::ptr event) {
    if (level >= m_level) {
        m_filestream << m_formatter.format(logger, level, event);
    }
}

void FileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter.format(logger, level, event);
    }

}

LogFormatter::LogFormatter(const std::string& pattern)
    : m_pattern(pattern) {

}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

void LogFormatter::init() {
    std::vector<std::tuple<std::string, std::string, int> > vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, m_pattern[i]);
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while (n < m_pattern.size()) {
            if (isspace(m_pattern[n])) {
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '(') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;//解析格式                
                    ++n;
                    fmt_begin = n;
                    continue;
                }
            }
            if (fmt_status == 1) {
                if (m_pattern[n] == ')') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 2;
                    break;
                }
            }
        }
        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
            }
            str = m_pattern.substr(i + 1, n - i - 1);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        } else if (fmt_status == 2) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
            }
            vec.push_back(std::make_tuple(str, fmt, 1)); 
            i = n;
        }                
    }
    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(str, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
    {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); }},
    XX(m, MessageFormatItem),
    XX(p, LevelFormatItem),
    XX(r, ElapseFormatItem),
    XX(c, NameFormatItem),
    XX(t, ThreadIdFormatItem),
    XX(n, NewLineFormatItem),
    XX(d, DateTimeFormatItem),
    XX(f, FilenameFormatItem),
    XX(l, LineFormatItem)
#undef XX
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        std::cout << std::get<0>(i) << " - " << std::get<1>(i) << " - " << std::get<2>(i) << endl;
    }
}



}

#endif 
