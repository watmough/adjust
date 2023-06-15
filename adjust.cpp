/*

# adjust - dynamic adjustment for system parameters

GNU/Linux CLI utility that permits easy dynamic adjustment for system
parameters such as brightness, gamma etc.

adjust

a tiny utility for adjusting attributes in the terminal.
usage: adjust <attribute>
       left/right cursor keys to decrement/increment
       q to quit

Function: display minimal text ui outputting saved/adjusted attribute 
value to a preconfigured command. q to end.

How it works: A configuration provides a value range, a step value, and
an initial value. This provides a value which can be manipulated by the
left / right cursor keys. 
After each change of value, the provided command is run with '%' being
replaced by the value.

Bugs/Issues: There is very little error checking, as this is really just
a personal tool
No configuration except by editing additional attributes into the
~/.adjustments configuration file.
If you want to run it as a normal user, you'll likely need to change the 
owner / permissions to SUID.

Examples of items that can be adjusted by this tool

1.  GNU/Linux XWindows Gamma Value
    Command to use: xgamma -gamma value
    Reasonable value range: 0.1 to 0.90 by 0.1 default 0.4

2.  NVidia-specific Brightness Command
    Command to use: echo value > /sys/class/backlight/nvidia_0/brightness
    Reasonable value range: 15 to 95 by 5 default 20

Example ~/.adjustments Configuration File - Place the following four lines
in ~/.adjustments

gamma 0.1 0.9 0.1 0.5
xgamma -gamma %
brightness 15 95 5 20
echo % > /sys/class/backlight/nvidia_0/brightness

Build Instructions
g++ adjust.cpp -lcurses -o adjust

To provide a regular user with superuser privileges for writing to /dev
items:

sudo chown root adjust
sudo chmod +s adjust

*/

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <iterator>
#include <ncurses.h>
#include <string>
#include <ostream>
#include <sstream>
#include <tuple>
#include <vector>

#define UP 'A'
#define DOWN 'B'
#define LEFT 'D'
#define RIGHT 'C'

typedef std::tuple<std::string, double, double, double, double, std::string> adjustment;
typedef std::vector<adjustment> adjustments;

double adjust(adjustment _adjustment)
{
    std::string attribute{std::get<0>(_adjustment)};
    double low{std::get<1>(_adjustment)};
    double high{std::get<2>(_adjustment)};
    double step{std::get<3>(_adjustment)};
    double value{std::get<4>(_adjustment)};
    std::string command{std::get<5>(_adjustment)};
    int key{0};
    do
    {
        // check for q for quit
        if (key == 'q')
            break;

        // skip initial part of escape sequence e.g 27 [ ABCD (cursor keys)
        if (key == 27) {
            // skip '['
            getc(stdin);
            continue;
        }

        // decrement / increment value with left / right cursor keys
        if (key == LEFT)
            value = std::max(value - step, low);
        if (key == RIGHT)
            value = std::min(value + step, high);

        // copy the command into the output ss with '%' replaced by value and 2>/dev/null appended
        std::stringstream output;
        std::copy(command.begin(), std::find(command.begin(), command.end(), '%'),
                  //   std::back_inserter(output));
                  std::ostream_iterator<char>(output));
        output << std::noshowpoint << value << std::showpoint;
        std::copy(std::find(command.begin(), command.end(), '%') + 1, command.end(),
                  //   std::back_inserter(output));
                  std::ostream_iterator<char>(output));
        output << std::string(" 2>/dev/null");

        // reflect the adjusted value at the terminal as the user adjusts it
        printf("%s : %f\r", attribute.c_str(), value);

        // run the command
        system(output.str().c_str());
    }
    while ((key = getc(stdin)) != EOF);

    // return last value set, just in case we want to write the last adjustment into .local or similar
    return value;
}

// read the settings for named attribute / adjustment from ~/.adjustments
bool getadjustment(const std::string _attribute, adjustment &_adjustment)
{
    // read file .adjustments from users home folder
    char attribute[1024];
    char command[1024];
    double low, high, step, initial;
    FILE *f = fopen("/home/jonathan/.adjustments", "r");
    if (!f)
        goto out;
    while (!feof(f)) {
        // read: attribute low high step initial
        // read: command
        int fieldsread = fscanf(f, "%s%lf%lf%lf%lf\r%[-a-zA-Z0-9%/ _><]",
                                attribute, &low, &high, &step, &initial, command);

        // if you have a problem here, check the list of permitted characters in the
        // fscanf [...]. As writting I'm using %[-a-zA-Z0-9%/ _><]

        // is this the attribute you're looking for
        if (fieldsread == 6 && _attribute == attribute) {
            // yup instantiate a tuple and return it
            _adjustment = std::make_tuple(attribute, low, high, step, initial, command);
            return true;
        }
    }
    fclose(f);
out:
    return false;
}

int main(int argc, char *argv[])
{
    // read args
    if (argc!=2) {
        // pick up a list of available attributes / print usage
        printf( "\n    adjust - dynamic adjustment for system parameters\n"
                "\n"
                "GNU/Linux CLI utility that permits easy dynamic adjustment for system\n"
                "parameters such as brightness, gamma etc.\n"
                "\n"
                "adjust\n"
                "\n"
                "a tiny utility for adjusting attributes in the terminal.\n"
                "usage: adjust <attribute>\n"
                "       left/right cursor keys to decrement/increment\n"
                "       q to quit\n"
                "\n"
                "Function: display minimal text ui outputting saved/adjusted attribute \n"
                "value to a preconfigured command. q to end.\n"
                "\n"
                "How it works: A configuration provides a value range, a step value, and\n"
                "an initial value. This provides a value which can be manipulated by the\n"
                "left / right cursor keys. \n"
                "After each change of value, the provided command is run with '%' being\n"
                "replaced by the value.\n"
                "\n"
                "Bugs/Issues: There is very little error checking, as this is really just\n"
                "a personal tool\n"
                "No configuration except by editing additional attributes into the\n"
                "~/.adjustments configuration file.\n"
                "If you want to run it as a normal user, you'll likely need to change the \n"
                "owner / permissions to SUID.\n"
                "\n"
                "Examples of items that can be adjusted by this tool\n"
                "\n"
                "1.  GNU/Linux XWindows Gamma Value\n"
                "    Command to use: xgamma -gamma value\n"
                "    Reasonable value range: 0.1 to 0.90 by 0.1 default 0.4\n"
                "\n"
                "2.  NVidia-specific Brightness Command\n"
                "    Command to use: echo value > /sys/class/backlight/nvidia_0/brightness\n"
                "    Reasonable value range: 15 to 95 by 5 default 20\n"
                "\n"
                "Example ~/.adjustments Configuration File - Place the following four lines\n"
                "in ~/.adjustments\n"
                "\n"
                "gamma 0.1 0.9 0.1 0.5\n"
                "xgamma -gamma %\n"
                "brightness 15 95 5 20\n"
                "echo % > /sys/class/backlight/nvidia_0/brightness\n"
                "\n"
                "Build Instructions\n"
                "g++ adjust.cpp -lcurses -o adjust\n"
                "\n"
                "To provide a regular user with superuser privileges for writing to /dev\n"
                "items:\n"
                "\n"
                "sudo chown root adjust\n"
                "sudo chmod +s adjust\n\n");
        exit(1);
    } else {
        // init curses and get chars as they come in
        // https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/helloworld.html
        initscr();
        cbreak();

        // read attribute name
        adjustment _adjustment;
        if (getadjustment(argv[1], _adjustment)) {
            adjust(_adjustment);
        }

        // clear out the current line, then CR to cursor back to the start
        int _,cols;
        getyx(stdscr, _, cols);
        printf("%s\r", std::string(cols, ' '));

        // back to normal terminal behavior
        endwin();
    }

    return 0;
}
