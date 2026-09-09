#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026, lnc.
# Developer : Mohammed Al-Baqer

import logging
from typing import Any, Dict, List, Optional
from fastapi import APIRouter, HTTPException
from pydantic import BaseModel, Field
from ..utils.config import Config

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/api/settings", tags=["settings"])

# Initialize configuration
config = Config()
class SettingsUpdate(BaseModel):
    updates: Dict[str, Any] = Field(..., description="Settings to update")

@router.get("")
async def get_settings() -> Dict[str, Any]:
    try:
        return config.get_all()
    except Exception as e:
        logger.error(f"Failed to get settings: {e}")
        raise HTTPException(status_code=500, detail="Failed to get settings")

@router.put("")
async def update_settings(request: SettingsUpdate) -> Dict[str, Any]:
    try:
        config.update(request.updates)
        return {
            'success': True,
            'message': 'Settings updated successfully',
            'settings': config.get_all(),
        }
    except Exception as e:
        logger.error(f"Failed to update settings: {e}")
        raise HTTPException(status_code=500, detail="Failed to update settings")

@router.post("/reset")
async def reset_settings() -> Dict[str, Any]:
    try:
        config.reset()
        return {
            'success': True,
            'message': 'Settings reset to defaults',
            'settings': config.get_all(),
        }
    except Exception as e:
        logger.error(f"Failed to reset settings: {e}")
        raise HTTPException(status_code=500, detail="Failed to reset settings")