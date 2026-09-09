#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

from .routes_system import router as system_router
from .routes_process import router as process_router
from .routes_disk import router as disk_router
from .routes_network import router as network_router
from .routes_services import router as services_router
from .routes_settings import router as settings_router

__all__ = [
    'system_router',
    'process_router',
    'disk_router',
    'network_router',
    'services_router',
    'settings_router',
]