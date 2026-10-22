# Hypervisor

Code for the hypervisor I am using to research defenses based on virtualization for my PhD.

Written in modern C++, it's based on the wonderful tutorial series from [Rayanfam Blog](https://rayanfam.com/topics/hypervisor-from-scratch-part-1/), with minor tweaks to work on latest version of Windows 11 and structural improvements offered by the latest C++ versions.

It's a thin Intel VT-x hypervisor running as a Windows driver, virtualizing the current OS. Works in Hyper-V.

Current status: complete virtualization of Windows 11, vmcall support, devirtualization via CPUID

Next steps: EPT, modular architecture, anti-exploit modules
