#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

__version__ = "1.0.0"
__author__ = "Developer System Monitor Team"

from .core.native import NativeInterface
from .core.system_manager import SystemManager

__all__ = ['NativeInterface', 'SystemManager', '__version__']