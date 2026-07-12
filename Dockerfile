# Production Dockerfile for SA-MP/OpenMP Server
FROM ubuntu:24.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install required system dependencies
# Note: OpenMP server is 32-bit, so we need 32-bit compatibility libraries
RUN dpkg --add-architecture i386 && \
    apt-get update && apt-get install -y \
    libmariadb3 \
    libmariadb-dev \
    libssl3 \
    ca-certificates \
    wget \
    curl \
    procps \
    libc6-i386 \
    lib32gcc-s1 \
    libatomic1:i386 \
    lib32stdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user for security
# Check if user 1000 exists, if so use it, otherwise create new user
RUN if id 1000 >/dev/null 2>&1; then \
        userdel -r $(id -un 1000) 2>/dev/null || true; \
    fi && \
    useradd -m -u 1000 -s /bin/bash samp && \
    mkdir -p /samp && \
    chown -R samp:samp /samp

# Set working directory
WORKDIR /samp

# Copy local omp-server binary
COPY --chown=samp:samp omp-server /samp/omp-server
RUN chmod +x /samp/omp-server && \
    echo "✓ Using local omp-server binary"

# Copy log-core runtime (required by the mysql plugin) from root
COPY --chown=samp:samp log-core.so /samp/
COPY --chown=samp:samp log-core2.so /samp/

# Copy plugins (Linux .so files only, no .dll files)
COPY --chown=samp:samp plugins/*.so /samp/plugins/

# Copy gamemodes (optional - can be overridden with volume mounts)
# COPY --chown=samp:samp gamemodes/*.amx /samp/gamemodes/

# Copy filterscripts (optional - can be overridden with volume mounts)
# COPY --chown=samp:samp filterscripts/*.amx /samp/filterscripts/

# Create directories for mounted volumes (will be overridden by volume mounts)
RUN mkdir -p /samp/gamemodes /samp/filterscripts && \
    chown -R samp:samp /samp/gamemodes /samp/filterscripts

# Copy scriptfiles
COPY --chown=samp:samp scriptfiles/ /samp/scriptfiles/

# Copy config files
COPY --chown=samp:samp config.json /samp/
COPY --chown=samp:samp bans.json /samp/

# Copy entrypoint script
COPY --chown=samp:samp docker-entrypoint.sh /samp/
RUN chmod +x /samp/docker-entrypoint.sh

# Copy components directory
COPY --chown=samp:samp components/ /samp/components/

# Create necessary directories
RUN mkdir -p /samp/logs /samp/saves && \
    chown -R samp:samp /samp

# Switch to non-root user
USER samp

# Expose server port (default 7777, but configurable via config.json)
EXPOSE 7777/udp

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD pgrep omp-server > /dev/null || exit 1

# Use entrypoint script for flexible configuration
ENTRYPOINT ["/samp/docker-entrypoint.sh"]
