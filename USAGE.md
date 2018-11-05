# AnyCollect Usage

## Contents
- [AnyCollect Usage](#anycollect-usage)
  - [Contents](#contents)
  - [Metrics](#metrics)
    - [Sources](#sources)
    - [Expressions](#expressions)
    - [Metrics and substitution](#metrics-and-substitution)
    - [Examples](#examples)
      - [Basic matching](#basic-matching)
      - [Multiple metrics on one line:](#multiple-metrics-on-one-line)
  - [AnyCollectValues Standalone interface](#anycollectvalues-standalone-interface)
  - [Snap Configuration](#snap-configuration)
    - [Global configuration](#global-configuration)
    - [Configuration](#configuration)
    - [Parameters](#parameters)
    - [Metrics](#metrics-1)

## Metrics
AnyCollect can gather metrics from two different kinds of sources:
 * Reading files from the filesystem
 * Executing commands

Each of these sources have an array of expressions which are used to filter and match the contents. Each expression is a regex and an array of metrics templates, which use regex matches to form metrics.

### Sources
Metrics are defined in a JSON file:
```json
{
  "Files": [
    {
      "Paths": [
        ""
      ],
      "Expressions": [...]
    }
  ]
  "Commands": [
    {
      "Program": "",
      "Arguments": [
        ""
      ],
      "Expressions": [...]
    }
  ],
}
```

Metrics from reading file contents are specified in the top-level `Files` array. Each require two fields:
 - `Paths`, an array of strings representing the paths of files to read
 - `Expressions`, an array of expressions (see [below](#expressions)) used to match file contents

Paths support standard POSIX globbing (with wildcards, etc.). You can specify multiple paths to be matched against the same expressions.

Metrics from command output are specified in the top-level `Command` array. It requires three fields:
 - `Program`, the main program to execute
 - `Arguments`, an array of additional arguments to give to the program
 - `Expressions`, an array of expressions (see [below](#expressions)) used to match command output


### Expressions
An expression is defined by two fields:
 - `Regex`, a regular expression string (backslashes `\` and quotes `"` should be escaped)
 - `Metrics`, an array of metric templates

A metric template is in turn defined in JSON by six fields:
 - `Name`, an array of strings representing the name of the metric
 - `Value`, a string
 - `Unit`, a string
 - `Tags`, a map associating strings to strings
 - `ComputeRate`, a boolean indicating whether the variation of the value, rather than the value, should be collected
 - `ConvertToUnitsPerSecond`, a boolean indicating whether the value should be converted to units per second

The regex will be applied for each line of content. If a match is found, metric templates are attempted to be filled with variable substitution.

One expression may have more than one metric template to facilitate parsing: if a line contains multiple metrics, the whole line can be matched by the regex and each metric template will extract one metric from the regex match.


### Metrics and substitution
String fields of a metric template (`Name`, `Value`, `Unit` and `Tags`) are subject to variable substitution:
 - `$1`, `$2`, `$3`, etc. will be replaced by the regex submatch with the same index. `$0` matches the whole match
 - `$path_0`, `$path_1`, `$path_2`, etc. will be replaced by the path component at the same index

**After substitution, a metric is defined by its `Name` and `Tags` fields**: if two metrics have the same `Name` and `Tags` fields, they are considered to represent the same thing. This equivalence is used to compute rates from an iteration to the next, and in case two metrics are found to be equivalent during the same iteration then their values are added.

In the end, **the `Value` field must be convertible to a number** (either integer or floating point).

**If any of `Name`, `Value`, or `Tags` field is empty after substitution, the metric is considered deficient and is dropped.**


### Examples
**Please refer to the examples config files in the `example` directory of this repo.**

#### Basic matching
Memory stats in `/proc/meminfo`
```
MemTotal: 13192071 kB
```

Regex: `^(\w+) (\d+) (\w)B$`

Matches:
| Index | Submatch                  |
|-------|---------------------------|
| 0     | `MemTotal: 13192071 kB` |
| 1     | `MemTotal`                     |
| 2     | `13192071`                    |
| 3     | `k`                     |

Metrics:
| Template | Metric|
|----------|-------|
| <ul style="list-style: none;"> <li>Name: `["memory", "$1"]`,</li> <li>Value: `"$2"`,</li> <li>Unit: `"$3B"`,</li> <li>Tags: `{}`</li> </ul>   | <ul style="list-style: none;"> <li>Name: `["memory", "MemTotal"]`,</li> <li>Value: `13192071`,</li> <li>Unit: `"kB"`,</li> <li>Tags: `{}`</li> </ul>    |


#### Multiple metrics on one line:
CPU stats in `/proc/stat`
```
cpu  1357 9 224 1307735 4 0 3 0 0
```

Regex: `^cpu  (\d+) \d+ (\d+) (\d+)`

Matches:
| Index | Submatch                  |
|-------|---------------------------|
| 0     | `cpu  1357 9 224 1307735` |
| 1     | `cpu`                     |
| 2     | `1357`                    |
| 3     | `224`                     |
| 4     | `1307735`                 |

Metrics:
| Template | Metric|
|----------|-------|
| <ul style="list-style: none;"> <li>Name: `["cpu", "user"]`,</li> <li>Value: `"$1"`,</li> <li>Unit: `"jiffies"`,</li> <li>Tags: `{}`</li> </ul>   | <ul style="list-style: none;"> <li>Name: `["cpu", "user"]`,</li> <li>Value: `1357`,</li> <li>Unit: `"jiffies"`,</li> <li>Tags: `{}`</li> </ul>    |
| <ul style="list-style: none;"> <li>Name: `["cpu", "system"]`,</li> <li>Value: `"$2"`,</li> <li>Unit: `"jiffies"`,</li> <li>Tags: `{}`</li> </ul> | <ul style="list-style: none;"> <li>Name: `["cpu", "system"]`,</li> <li>Value: `224`,</li> <li>Unit: `"jiffies"`,</li> <li>Tags: `{}`</li>  </ul>  |
| <ul style="list-style: none;"> <li>Name: `["cpu", "idle"]`,</li> <li>Value: `"$3"`,</li> <li>Unit: `"jiffies"`,</li> <li>Tags: `{}`</li> </ul>   | <ul style="list-style: none;"> <li>Name: `["cpu", "idle"]`,</li> <li>Value: `1307735`,</li> <li>Unit: `"jiffies"`,</li> <li>Tags: `{}`</li> </ul> |


## AnyCollectValues Standalone interface
This program collects all available metrics and prints them on the standard output. For each metric, it shows its name, its value and its unit.

AnyCollectValues can take up to **three arguments**:
- the sampling interval in second, that is the duration to wait between two readings of kernel values. Default is 1 second
- JSON configuration file path
- how many times to report metrics. Default is 0, which means unlimited time

For example, `./AnyCollectValues 60 ./config.json 10` will read the config file `./config.json`, and then collect metrics and print them every 60 seconds; it will do so 10 times. The program will thus run for 10 minutes.


## Snap Configuration
### Global configuration
In order for the AnyCollect plugin to be aware of its configuration before Snap launches a task (otherwise metrics won't be registered), the configuration file must be specified in snapteld global config:

```yaml
---
control:
  plugins:
    collector:
      anycollect:
        all:
          ConfigFile: /etc/snap/anycollect.json
```

### Configuration
First set up the [Snap framework](https://github.com/intelsdi-x/snap/blob/master/README.md#getting-started).

The default configuration for a AnyCollect Snap task is the following:
```yaml
---
  version: 1
  schedule:
    type: "streaming"
  max-failures: 10
  workflow:
    collect:
      metrics:
        /cfm/anycollect/*: {}
      config:
        /cfm:
          SamplingInterval: 1
          # MaxMetricsBuffer: 0
          # MaxCollectDuration: 0
      publish:
	    ...
```

### Parameters
The parameters are (default values are given [above](#configuration)):
 - `SamplingInterval` (type int): delay in seconds between two readings of the kernel values
 - `MaxMetricsBuffer` (type int): maximum number of metrics to send to Snap at once
 - `MaxCollectDuration` (type int): maximum waiting time (in seconds) before sending metrics to Snap


### Metrics
Collected metrics are described [above](#metrics).

Each metric template descibed in the configuration file will, after regex substitution, be transformed into a Snap metric. The metric name is converted into a metric namespace which you can filter in the Snap task file.

You can use wildcards to specify groups of metrics easily:
```
/cfm/anycollect/*: {}
```
