## ManPageQL

**TODO**

### Debugging & test

```
# for macOS >= 10.13
log stream --style compact --predicate 'process == "QuickLookSatellite" AND eventMessage CONTAINS "ManPageQL"' --color=auto

# for macOS >= 10.12
log stream --style compact --predicate 'process == "QuickLookSatellite" AND eventMessage CONTAINS "ManPageQL"'

# for macOS < 10.12
syslog -w 0 -k Sender QuickLookSatellite -k Message S ManPageQL
```

### *References*

[clang, change dependent shared library install name at link time](https://stackoverflow.com/questions/27506450/clang-change-dependent-shared-library-install-name-at-link-time)

[Install_name on OS X](http://log.zyxar.com/blog/2012/03/10/install-name-on-os-x/)

[Xcode Build Settings Reference](https://pewpewthespells.com/blog/buildsettings.html)

[Creating Dynamic Libraries](https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/DynamicLibraries/100-Articles/CreatingDynamicLibraries.html)

---

*Created 190219+0800*