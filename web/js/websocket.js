/**
 * WebSocket Client for real-time monitoring
 */

class WebSocketClient {
    constructor(url = null) {
        this.url = url || this.getDefaultURL();
        this.ws = null;
        this.connected = false;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 10;
        this.reconnectDelay = 2000;
        this.listeners = new Map();
        this.heartbeatInterval = null;
    }

    getDefaultURL() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const host = window.location.host || 'localhost:8000';
        return `${protocol}//${host}/ws/monitor`;
    }

    connect() {
        if (this.ws && (this.ws.readyState === WebSocket.OPEN || this.ws.readyState === WebSocket.CONNECTING)) {
            console.log('WebSocket already connected or connecting');
            return;
        }

        console.log(`Connecting to WebSocket: ${this.url}`);

        try {
            this.ws = new WebSocket(this.url);

            this.ws.onopen = () => {
                console.log('WebSocket connected');
                this.connected = true;
                this.reconnectAttempts = 0;
                this.startHeartbeat();
                this.emit('connected', {});
            };

            this.ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    this.emit('message', data);
                    this.emit('metrics', data);
                } catch (error) {
                    console.error('Failed to parse WebSocket message:', error);
                }
            };

            this.ws.onerror = (error) => {
                console.error('WebSocket error:', error);
                this.emit('error', { error });
            };

            this.ws.onclose = (event) => {
                console.log('WebSocket disconnected:', event.code, event.reason);
                this.connected = false;
                this.stopHeartbeat();
                this.emit('disconnected', { code: event.code, reason: event.reason });
                this.attemptReconnect();
            };
        } catch (error) {
            console.error('Failed to create WebSocket:', error);
            this.attemptReconnect();
        }
    }

    attemptReconnect() {
        if (this.reconnectAttempts >= this.maxReconnectAttempts) {
            console.error('Max reconnect attempts reached');
            this.emit('maxReconnectAttempts', {});
            return;
        }

        this.reconnectAttempts++;
        const delay = this.reconnectDelay * this.reconnectAttempts;

        console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts})`);

        setTimeout(() => {
            this.connect();
        }, delay);
    }

    startHeartbeat() {
        this.heartbeatInterval = setInterval(() => {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                this.ws.send(JSON.stringify({ type: 'ping', timestamp: Date.now() }));
            }
        }, 30000);
    }

    stopHeartbeat() {
        if (this.heartbeatInterval) {
            clearInterval(this.heartbeatInterval);
            this.heartbeatInterval = null;
        }
    }

    disconnect() {
        this.stopHeartbeat();
        if (this.ws) {
            this.ws.close(1000, 'Client disconnected');
            this.ws = null;
        }
        this.connected = false;
    }

    send(data) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(JSON.stringify(data));
        } else {
            console.warn('WebSocket not connected');
        }
    }

    on(event, callback) {
        if (!this.listeners.has(event)) {
            this.listeners.set(event, []);
        }
        this.listeners.get(event).push(callback);
    }

    off(event, callback) {
        if (this.listeners.has(event)) {
            const callbacks = this.listeners.get(event);
            const index = callbacks.indexOf(callback);
            if (index > -1) {
                callbacks.splice(index, 1);
            }
        }
    }

    emit(event, data) {
        if (this.listeners.has(event)) {
            const callbacks = this.listeners.get(event);
            callbacks.forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error(`WebSocket listener error for ${event}:`, error);
                }
            });
        }
    }

    isConnected() {
        return this.connected;
    }
}

const wsClient = new WebSocketClient();

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { WebSocketClient, wsClient };
}